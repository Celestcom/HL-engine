#include "stdafx.h"
#include "IHardwareDevice.h"
#include "PluginInstance.h"
#include "PluginApis.h"
#include "nsvr_preset.h"

#include <boost/log/trivial.hpp>
#include "Locator.h"
#include "HardwareCoordinator.h"
#include "BodyGraph.h"
#include "DeviceContainer.h"

#include <boost/variant/variant.hpp>

Device::Device(const DeviceDescriptor& descriptor, PluginApis& capi, PluginEventSource& ev, Parsing::BodyGraphDescriptor bodyGraph)
	: m_name(descriptor.displayName)
	, m_apis(&capi)
	, m_nodes()
	, m_isBodyGraphSetup(false)
	, m_deviceId(descriptor.id)
{
	
	
	dynamicallyFetchNodes();

	setupInitialBodyRepresentation(bodyGraph);
	setupDynamicBodyRepresentation();

}

void Device::dynamicallyFetchNodes()
{
	device_api* enumerator = m_apis->GetApi<device_api>();

	nsvr_node_ids device_ids = { 0 };
	enumerator->submit_enumeratenodes(id(), &device_ids);

	std::vector<NodeDescriptor> devices;
	devices.reserve(device_ids.node_count);

	for (std::size_t i = 0; i < device_ids.node_count; i++) {
		fetchNodeInfo(device_ids.ids[i]);
	}
}





void Device::parseNodes(const std::vector<NodeDescriptor>& descriptor)
{
	
	BOOST_LOG_TRIVIAL(info) << "[Device] " << m_name << " describes " << descriptor.size() << " devices:";
	for (const auto& node : descriptor) {
		createNewNode(node);
	}
}



void Device::fetchNodeInfo(uint64_t device_id) {
	device_api* enumerator = m_apis->GetApi<device_api>();
	nsvr_node_info info = { 0 };
	enumerator->submit_getnodeinfo(device_id, &info);

	NodeDescriptor desc;
	desc.capabilities = info.capabilities;
	desc.displayName = std::string(info.name);
	desc.id = device_id;
	desc.type = info.type;

	createNewNode(desc);

}

void Device::createNewNode(const NodeDescriptor& node)
{
	const auto& t = Locator::Translator();


	m_nodes.push_back(std::make_unique<Node>(node, m_apis));

	if (node.type == nsvr_node_type_haptic) {
		m_simulatedNodes.insert(std::make_pair(node.id, SimulatedHapticNode()));
	}

	if (node.type == nsvr_node_type_tracker) {
		m_trackedObjects.insert(std::make_pair(node.id, 
			std::make_unique<TrackingStream>(node.id, m_apis->GetApi<tracking_api>())));
	}

}




void Device::deliverRequest(const NullSpaceIPC::HighLevelEvent& event)
{
	switch (event.events_case()) {
	case NullSpaceIPC::HighLevelEvent::kSimpleHaptic:
		handleSimpleHaptic(event.parent_id(), event.simple_haptic());
		break;
	case NullSpaceIPC::HighLevelEvent::kPlaybackEvent:
		handlePlaybackEvent(event.parent_id(), event.playback_event());
		break;
	default:
		BOOST_LOG_TRIVIAL(info) << "[Device] Unrecognized request: " << event.events_case();
		break;
	}
}



void Device::handleSimpleHaptic(RequestId requestId, const NullSpaceIPC::SimpleHaptic& simple)
{
	if (auto api = m_apis->GetApi<waveform_api>()) {
		nsvr_waveform wave{};
		wave.repetitions = static_cast<std::size_t>(simple.duration() / 0.25f);
		wave.strength = simple.strength();
		wave.waveform_id = static_cast<nsvr_default_waveform>(simple.effect());

		for (uint64_t region : simple.regions()) {
			auto nodes = m_graph.getNodesForNamedRegion(static_cast<subregion::shared_region>(region));
			for (const auto& node : nodes) {
				m_simulatedNodes[node].submitHaptic(Waveform(requestId, wave.waveform_id, wave.strength, simple.duration()));
				api->submit_activate(requestId, node, reinterpret_cast<nsvr_waveform*>(&wave));
			}
		}
	} 

	else if (auto api = m_apis->GetApi<buffered_api>()) {
		double sampleDuration = 0;
		api->submit_getsampleduration(&sampleDuration);
		uint32_t numNecessarySamples = std::max<uint32_t>(1, static_cast<uint32_t>(simple.duration() / sampleDuration));

		std::vector<double> samples(numNecessarySamples, simple.strength());

		for (uint64_t region : simple.regions()) {
			auto nodes = m_graph.getNodesForNamedRegion(static_cast<subregion::shared_region>(region));
			for (const auto& node : nodes) {
				m_simulatedNodes[node].submitHaptic(Waveform(requestId, samples.data(), sampleDuration, samples.size()));

				api->submit_buffer(node, samples.data(), samples.size());
			}
		}
	}
	

	
}





void Device::handlePlaybackEvent(RequestId id, const ::NullSpaceIPC::PlaybackEvent& event)
{
	
		if (playback_api* api = m_apis->GetApi<playback_api>()) {
		switch (event.command()) {
		case NullSpaceIPC::PlaybackEvent_Command_UNPAUSE:
			api->submit_unpause(id);
			for (auto& node : m_simulatedNodes) {
				node.second.submitPlayback(id, SimulatedHapticNode::PlaybackCommand::Resume);
			}
			break;
		case NullSpaceIPC::PlaybackEvent_Command_PAUSE:
			api->submit_pause(id);
			for (auto& node : m_simulatedNodes) {
				node.second.submitPlayback(id, SimulatedHapticNode::PlaybackCommand::Pause);
			}
			break;
		case NullSpaceIPC::PlaybackEvent_Command_CANCEL:
			api->submit_cancel(id);
			for (auto& node : m_simulatedNodes) {
				node.second.submitPlayback(id, SimulatedHapticNode::PlaybackCommand::Cancel);
			}
			break;
		default:
			BOOST_LOG_TRIVIAL(warning) << "[Device] Unknown playback event: " << event.command();
			break;
		}
	}
}

//returns nullptr on failure
const Node * Device::findNode(nsvr_node_id id) const
{
	auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [id](const std::unique_ptr<Node>& node) {return node->id() == id; });
	if (it != m_nodes.end()) {
		return it->get();
	}
	return nullptr;
}

Node * Device::findNode(uint64_t id)
{
	return const_cast<Node*>(static_cast<const Device&>(*this).findNode(id));
}


class region_visitor : public boost::static_visitor<void> {
private:
	BodyGraph& m_graph;
public:
	region_visitor(BodyGraph& graph) : m_graph(graph) {

	}
	void operator()(const Parsing::SingleRegionDescriptor& single) {
		nsvr_bodygraph_region region;
		region.bodypart = single.bodypart;
		region.rotation = single.location.rotation;
		region.segment_ratio = single.location.height;
		m_graph.CreateNode(single.name.c_str(), &region);
	}

	void operator()(const Parsing::MultiRegionDescriptor& multi) {
		//not implemented yet
	}
};

void Device::setupInitialBodyRepresentation(const Parsing::BodyGraphDescriptor& bodyGraph)
{
	region_visitor visitor(m_graph);
	for (const auto& region : bodyGraph.regions) {
		boost::apply_visitor(visitor, region);
	}

}



Node::Node(const NodeDescriptor& description, PluginApis* apis)
	: m_id{ description.id }
	, m_name{ description.displayName }
	, m_apis(apis)
	, m_type(description.type)
	{}




nsvr_node_id Node::id() const
{
	return m_id;
}

std::string Node::name() const
{
	return m_name;
}

nsvr_node_type Node::type() const
{
	return m_type;
}


std::string Device::name() const
{
	return m_name;
}



void Device::registerTrackedObjects(const boost::signals2::signal<void(nsvr_node_id, nsvr_quaternion*)>::slot_type& slot)
{
	boost::signals2::signal<void(nsvr_node_id, nsvr_quaternion*)> t;

	for (auto& node : m_nodes) {
		if (node->type() == nsvr_node_type_tracker) {
			m_trackedObjects[node->id()]->Signal.connect(slot);
		}
	}
}


void Device::setupDynamicBodyRepresentation()
{
	bodygraph_api* b = m_apis->GetApi<bodygraph_api>();
	if (b != nullptr) {
		
		b->submit_setup(reinterpret_cast<nsvr_bodygraph*>(&m_graph));
		m_isBodyGraphSetup.store(true);
	}
}





std::vector<NodeView> Device::renderDevices() const
{
	if (!m_isBodyGraphSetup.load()) {
		return std::vector<NodeView>{};
	}
	

	auto devices = m_graph.getAllNodes();
	std::vector<NodeView> fullView;
	for (const auto& kvp : devices) {
		NodeView view;
		view.region = static_cast<uint64_t>(kvp.first);
		
		for (const auto& id : kvp.second) {
			NodeView::SingleNode singleNode;
			
			const Node* possibleNode = findNode(id);
			
			if (possibleNode) {
				double strength = m_simulatedNodes.at(id).sample();
				singleNode.DisplayData.intensity = static_cast<float>(strength);
				singleNode.Id = id;
				singleNode.Type = NodeView::NodeType::Haptic;
				view.nodes.push_back(singleNode);

			
			}

		}

		fullView.push_back(view);

	}
	
	return fullView;
}



void Device::simulate(double dt)
{
	for (auto& node : m_simulatedNodes) {
		node.second.update(dt);
	}
}

uint32_t Device::id() const
{
	return m_deviceId;
}

TrackingStream::TrackingStream(nsvr_node_id id, tracking_api*  api)
	: m_api(api)
	, m_id(id)
{
	m_api->submit_beginstreaming(reinterpret_cast<nsvr_tracking_stream*>(this), m_id);
}

TrackingStream::~TrackingStream()
{
	m_api->submit_endstreaming(m_id);
}

void TrackingStream::deliver(nsvr_quaternion* q)
{
	//Note that there is no queue, tracking is delivered directly from the given plugin into shared memory 
	//we need synchronization for this, of course. Do we have it? I don't think so
	//todo: verify we have sync
	Signal(m_id, q);
}
