#include "stdafx.h"
#include "IHardwareDevice.h"
#include "PluginInstance.h"
#include "PluginApis.h"
#include "nsvr_preset.h"

#include <boost/log/trivial.hpp>


std::unique_ptr<NodalDevice> device_factories::createDevice(const HardwareDescriptor& description, PluginApis& cap, PluginEventHandler& ev)
{
	std::unique_ptr<NodalDevice> device{};
	switch (description.concept) {
	case HardwareDescriptor::Concept::Suit:
		device = std::unique_ptr<NodalDevice>(new SuitDevice{description, cap, ev});
		break;
	case HardwareDescriptor::Concept::Controller:
		device = std::unique_ptr<NodalDevice>(new ControllerDevice{description, cap, ev});
		break;
	default:
		BOOST_LOG_TRIVIAL(info) << "[HardwareDeviceFactory] Unrecognized concept: " << (int)description.concept << " in device " << description.displayName;
		break;
	}

	return device;
}

NodalDevice::NodalDevice(HardwareDescriptor& descriptor, PluginApis& capi, PluginEventHandler& ev)
	: m_name(descriptor.displayName)
	, m_apis(capi)
{

	setupSubscriptions(ev);
	
	if (!m_apis.SupportsApi("device")) {
		parseDevices(descriptor.nodes);
	}
	else {
		fetchDynamicDevices();
	}



	
}

void NodalDevice::setupSubscriptions(PluginEventHandler & ev)
{
	ev.Subscribe(nsvr_device_event_device_connected, [&](const nsvr::pevents::device_event& ev) {
		if (auto event = ev.get<nsvr::pevents::connected_event>()) {
			handle_connect(*event);
		}
	});

	ev.Subscribe(nsvr_device_event_device_disconnected, [&](const nsvr::pevents::device_event& ev) {
		if (auto event = ev.get<nsvr::pevents::disconnected_event>()) {
			handle_disconnect(*event);
		}
	});
}

void NodalDevice::parseDevices(const std::vector<NodeDescriptor>& descriptor)
{
	BOOST_LOG_TRIVIAL(info) << "[NodalDevice] " << m_name << " describes " << descriptor.size() << " devices:";
	for (const auto& node : descriptor) {
		if (NodeDescriptor::NodeType::Haptic == node.nodeType) {
			BOOST_LOG_TRIVIAL(info) << "[NodalDevice] Haptic node '" << node.displayName << "' on region " << node.region;
			this->addNode(std::unique_ptr<Node>(new HapticNode{ node, m_apis }));
		}
		else if (NodeDescriptor::NodeType::Tracker == node.nodeType) {
			BOOST_LOG_TRIVIAL(info) << "[NodalDevice] Tracking node '" << node.displayName << "' on region " << node.region;
			this->addNode(std::unique_ptr<Node>(new TrackingNode{ node, m_apis }));
		}
	}
}


void NodalDevice::addNode(std::unique_ptr<Node> node)
{
	//m_nodes assumes ownership of the node
	m_nodes.push_back(std::move(node));

	//we store a pointer to it in our nodesByRegion map
	auto region = m_nodes.back()->getRegion();
	m_nodesByRegion[region].push_back(m_nodes.back().get());
}

void NodalDevice::fetchDynamicDevices()
{
	device_api* enumerator = m_apis.GetApi<device_api>();

	nsvr_device_ids device_ids = { 0 };
	enumerator->submit_enumerateids(&device_ids);

	std::vector<NodeDescriptor> nodes;
	nodes.reserve(device_ids.device_count);

	for (std::size_t i = 0; i < device_ids.device_count; i++) {
		nsvr_device_basic_info info = { 0 };
		enumerator->submit_getinfo(device_ids.ids[i], &info);

		NodeDescriptor desc;
		desc.capabilities = info.capabilities;
		desc.displayName = info.name;
		desc.id = device_ids.ids[i];
		desc.nodeType = static_cast<NodeDescriptor::NodeType>(info.type);
		desc.region = info.region;

		nodes.push_back(desc);
	}

	parseDevices(nodes);
}



void NodalDevice::handle_connect(const nsvr::pevents::connected_event &e)
{
	BOOST_LOG_TRIVIAL(info) << "[NodalDevice][" << m_name << "] A device was connected!";
}

void NodalDevice::handle_disconnect(const nsvr::pevents::disconnected_event &)
{
	BOOST_LOG_TRIVIAL(info) << "[NodalDevice][" << m_name << "] A device was disconnected!";
}


void NodalDevice::deliverRequest(const NullSpaceIPC::HighLevelEvent& event)
{
	switch (event.events_case()) {
	case NullSpaceIPC::HighLevelEvent::kSimpleHaptic:
		handleSimpleHaptic(event.parent_id(), event.simple_haptic());
		break;
	case NullSpaceIPC::HighLevelEvent::kPlaybackEvent:
		handlePlaybackEvent(event.parent_id(), event.playback_event());
		break;
	default:
		BOOST_LOG_TRIVIAL(info) << "[NodalDevice] Unrecognized request: " << event.events_case();
		break;
	}
}



void NodalDevice::handleSimpleHaptic(RequestId requestId, const NullSpaceIPC::SimpleHaptic& simple)
{
	for (const auto& region : simple.regions()) {
		auto ev = nsvr::cevents::LastingHaptic(simple.effect(), simple.strength(), simple.duration(), region.c_str());

		if (auto api = m_apis.GetApi<request_api>()) {
			ev.handle = requestId;
			api->submit_request(reinterpret_cast<nsvr_request*>(&ev));
		}

		else if (auto api = m_apis.GetApi<preset_api>()) {
			auto& interested = m_nodesByRegion[region];
			for (auto& node : interested) {
				node->deliver(requestId, ev);
			}
		}
	}
}



Node * NodalDevice::findDevice(uint64_t id)
{
	for (auto& node : m_nodes) {
		if (node->id() == id) {
			return node.get();
		}
	}
	return nullptr;
}

void NodalDevice::handlePlaybackEvent(RequestId id, const ::NullSpaceIPC::PlaybackEvent& event)
{
	if (playback_api* api = m_apis.GetApi<playback_api>()) {
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
			BOOST_LOG_TRIVIAL(warning) << "[NodalDevice] Unknown playback event: " << event.command();
			break;
		}
	}
}




HapticNode::HapticNode(const NodeDescriptor& info, PluginApis &c) 
	: Node { info.id, info.displayName, info.capabilities }
	, m_apis(c)
	, m_region{info.region}

{

}

void HapticNode::deliver(RequestId, const nsvr::cevents::request_base & base)
{
	if (base.type() == nsvr_request_type_lasting_haptic) {
		const auto lasting = static_cast<const nsvr::cevents::LastingHaptic&>(base);
		if (auto api = m_apis.GetApi<buffered_api>()) {
		
			//api->submit_buffer()
		}
		else if (auto api = m_apis.GetApi<preset_api>()) {
			nsvr_preset_request req{};
			req.family = static_cast<nsvr_preset_family>(lasting.effect);
			req.strength = lasting.strength;
			api->submit_preset(reinterpret_cast<nsvr_preset_request*>(&req));
		}
	}
}

std::string HapticNode::getRegion() const
{
	return m_region;
}

SuitDevice::SuitDevice(HardwareDescriptor desc, PluginApis & capi, PluginEventHandler& ev)
	: NodalDevice{desc, capi, ev}
{

	if (tracking_api* api = capi.GetApi<tracking_api>()) {
		//api->
	}
	ev.Subscribe(nsvr_device_event_tracking_update, [&](const nsvr::pevents::device_event& ev) {
		if (auto event = ev.get<nsvr::pevents::tracking_event>()) {
			handle_tracking(*event);
		}
		else {
			BOOST_LOG_TRIVIAL(error) << "[SuitDevice] Tried to get a tracking event out of a device event, but it failed..";
		}
	});

}

void SuitDevice::handle_tracking(const nsvr::pevents::tracking_event&)
{
	
}

Node::Node(uint64_t id, const std::string& name, uint32_t capability) :m_id(id), m_name{ name }, m_capability(capability) {}


uint64_t Node::id() const
{
	return m_id;
}

std::string Node::name() const
{
	return m_name;
}

ControllerDevice::ControllerDevice(HardwareDescriptor desc, PluginApis & cap, PluginEventHandler & ev)
	: NodalDevice{desc, cap, ev}
{
}

std::string NodalDevice::name() const
{
	return m_name;
}

TrackingNode::TrackingNode(const NodeDescriptor & info, PluginApis & capi)
	: Node(info.id, info.displayName, info.capabilities)
	, m_region(info.region)
	, m_apis(capi)
{

	

}

void TrackingNode::deliver(RequestId, const nsvr::cevents::request_base &)
{

}

std::string TrackingNode::getRegion() const
{
	return m_region;
}
