#include "stdafx.h"
#include "IHardwareDevice.h"
#include "PluginInstance.h"
#include "PluginApis.h"
#include "nsvr_preset.h"

#include <boost/log/trivial.hpp>
#include "Locator.h"
#include "HardwareCoordinator.h"
#include "HumanBodyNodes.h"
#include "BodyGraph.h"
Device::Device(const DeviceDescriptor& descriptor, PluginApis& capi, PluginEventSource& ev)
	: m_name(descriptor.displayName)
	, m_apis(&capi)
	, m_trackingDevices()
	, m_hapticDevices()
	, m_isBodyGraphSetup(false)
	, m_systemId(descriptor.id)
{
	
	if (!m_apis->Supports<device_api>()) {
		//uh oh? Make it required
	}
	else {
		dynamicallyFetchNodes();
	}


	
}



void Device::createNewNode(const NodeDescriptor& node)
{
	const auto& t = Locator::Translator();

	if (NodeDescriptor::NodeType::Haptic == node.type) {
		//todo: fix
		//BOOST_LOG_TRIVIAL(info) << "[Device] Haptic node '" << node.displayName << "' on region " << t.ToRegionString(static_cast<nsvr_region>(node.region));
		m_hapticDevices.push_back(std::make_unique<HapticNode>(node, m_apis));
	}
	else if (NodeDescriptor::NodeType::Tracker == node.type) {
		//BOOST_LOG_TRIVIAL(info) << "[Device] Tracking node '" << node.displayName << "' on region " << t.ToRegionString(static_cast<nsvr_region>(node.region));
		m_trackingDevices.push_back(std::make_unique<TrackingNode>(node, m_apis));

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
	desc.displayName = info.name;
	desc.id = device_id;
	desc.type = static_cast<NodeDescriptor::NodeType>(info.type);

	createNewNode(desc);

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



void Device::handle_connect(uint64_t device_id)
{
	if (std::find(m_knownIds.begin(), m_knownIds.end(), device_id) == m_knownIds.end()) {
		BOOST_LOG_TRIVIAL(info) << "[Device][" << m_name << "] A new device was connected";
		m_knownIds.push_back(device_id);
		fetchNodeInfo(device_id);
	}
	else {
		BOOST_LOG_TRIVIAL(info) << "[Device][" << m_name << "] An already known device was reconnected";

	}
}

void Device::handle_disconnect(uint64_t device_id)
{
	auto it = std::find(m_knownIds.begin(), m_knownIds.end(), device_id);
	if (it != m_knownIds.end()) {
		m_knownIds.erase(it);
		BOOST_LOG_TRIVIAL(info) << "[Device][" << m_name << "] A known device was disconnected";
	}else {
		BOOST_LOG_TRIVIAL(info) << "[Device][" << m_name << "] An unknown device was disconnected";
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

	
	for (uint64_t region : simple.regions()) {
		
	

		
		auto devices = m_graph.getDevicesForNamedRegion(static_cast<subregion::shared_region>(region));


		if (auto api = m_apis->GetApi<waveform_api>()) {
			nsvr_waveform wave{};
			wave.repetitions = static_cast<std::size_t>(simple.duration() / 0.25f);
			wave.strength = simple.strength();
			wave.waveform_id = static_cast<nsvr_default_waveform>(simple.effect());
			for (const auto& device :devices) {
				api->submit_activate(requestId, device, reinterpret_cast<nsvr_waveform*>(&wave));
			}
		}
		else if (auto api = m_apis->GetApi<buffered_api>()) {

			double sampleDuration = 0;
			api->submit_getsampleduration(&sampleDuration);

			uint32_t numNecessarySamples = std::max<uint32_t>(1, static_cast<uint32_t>(simple.duration() / sampleDuration));

			std::vector<double> samples(numNecessarySamples, simple.strength());

			for (const auto& device : devices) {
				api->submit_buffer(device, samples.data(), samples.size());
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
			break;
		case NullSpaceIPC::PlaybackEvent_Command_PAUSE:
			api->submit_pause(id);
			break;
		case NullSpaceIPC::PlaybackEvent_Command_CANCEL:
			api->submit_cancel(id);
			break;
		default:
			BOOST_LOG_TRIVIAL(warning) << "[Device] Unknown playback event: " << event.command();
			break;
		}
	}
}

//returns nullptr on failure
Node * Device::findDevice(uint64_t id)
{
	auto it = std::find_if(m_hapticDevices.begin(), m_hapticDevices.end(), [id](const std::unique_ptr<HapticNode>& node) {return node->id() == id; });
	if (it != m_hapticDevices.end()) {
		return it->get();
	}

	auto it2 = std::find_if(m_trackingDevices.begin(), m_trackingDevices.end(), [id](const std::unique_ptr<TrackingNode>& node) { return node->id() == id; });
	if (it2 != m_trackingDevices.end()) {
		return it2->get();
	}

	return nullptr;
}




HapticNode::HapticNode(const NodeDescriptor& info, PluginApis*c) 
	: Node { info}
	, m_apis(c)


{

}



NodeView::Data HapticNode::Render() const
{
	if (sampling_api* api = m_apis->GetApi<sampling_api>()) {
		nsvr_sampling_sample state = { 0 };
		api->submit_query(m_region, &state);	
		return NodeView::Data { state.data_0, state.data_1, state.data_2, state.intensity };
	}

	return NodeView::Data { 0, 0, 0, 0 };
}

NodeView::NodeType HapticNode::Type() const
{
	return NodeView::NodeType::Haptic;
}



uint64_t HapticNode::Id() const
{
	return m_id;
}

Node::Node(const NodeDescriptor& description)
	: m_id{ description.id }
	, m_name{ description.displayName }
	, m_capability{ description.capabilities }
	{}


uint64_t Node::id() const
{
	return m_id;
}

std::string Node::name() const
{
	return m_name;
}

nsvr_region Node::region() const {
	return m_region;
}

std::string Device::name() const
{
	return m_name;
}

bool Device::hasCapability(Apis name) const
{
	return m_apis->Supports(name);
}

void Device::setupHooks(HardwareCoordinator & coordinator)
{
	if (m_apis->Supports<tracking_api>()) {
		for (auto& node : m_trackingDevices) {
			coordinator.Hook_TrackingSlot(node->TrackingSignal);
		}
	}
}

void Device::teardownHooks() {
	for (auto& node : m_trackingDevices) {
		node->TrackingSignal.disconnect_all_slots();
	}
	
}

void Device::setupBodyRepresentation(HumanBodyNodes & body)
{
	bodygraph_api* b = m_apis->GetApi<bodygraph_api>();
	if (b != nullptr) {
		
		b->submit_setup(reinterpret_cast<nsvr_bodygraph*>(&m_graph));
		m_isBodyGraphSetup.store(true);
	}
}

void Device::teardownBodyRepresentation(HumanBodyNodes & body)
{
	for (auto& node : m_hapticDevices) {
		body.RemoveNode(node.get());
	}
}




std::vector<NodeView> Device::renderDevices()
{
	if (!m_isBodyGraphSetup.load()) {
		return std::vector<NodeView>{};
	}


	auto devices = m_graph.getAllDevices();
	std::vector<NodeView> fullView;
	for (const auto& kvp : devices) {
		NodeView view;
		view.region = static_cast<uint64_t>(kvp.first);
		
		for (const auto& id : kvp.second) {
			NodeView::SingleNode singleNode;
			
			Node* possibleNode = findDevice(id);
			if (possibleNode) {
				Renderable* possibleRenderable = dynamic_cast<Renderable*>(possibleNode);
				if (possibleRenderable) {
					singleNode.DisplayData = possibleRenderable->Render();
					singleNode.Id = id;
					singleNode.Type = possibleRenderable->Type();
					view.nodes.push_back(singleNode);
				}
			}

		}

		fullView.push_back(view);

	}
	
	return fullView;
}



uint32_t Device::id() const
{
	return m_systemId;
}

TrackingNode::TrackingNode(const NodeDescriptor & info, PluginApis* capi)
	: Node(info)
	, m_apis(capi)
{

	BeginTracking();

}




void TrackingNode::BeginTracking()
{
	tracking_api* api = m_apis->GetApi<tracking_api>();
	api->submit_beginstreaming(reinterpret_cast<nsvr_tracking_stream*>(this), m_region);


	
}

void TrackingNode::EndTracking()
{
	tracking_api* api = m_apis->GetApi<tracking_api>();
	api->submit_endstreaming(m_region);
}

void TrackingNode::DeliverTracking(nsvr_quaternion * quat)
{
	m_latestQuat = *quat;
	TrackingSignal(m_region, &m_latestQuat);
}
