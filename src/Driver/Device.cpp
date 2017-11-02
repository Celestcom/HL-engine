#include "stdafx.h"
#include "Device.h"
#include "nsvr_region.h"
#include "protobuff_defs/HighLevelEvent.pb.h"
#include "logger.h"

Device::Device(
	std::string parentPlugin,
	DeviceDescriptor descriptor,
	std::unique_ptr<DeviceVisualizer> visualizer,
	std::unique_ptr<HardwareBodygraphCreator> bodygraph,
	std::unique_ptr<HardwareNodeEnumerator> discoverer,
	std::unique_ptr<HardwarePlaybackController> playback,
	std::unique_ptr<HardwareHapticInterface> haptics,
	std::unique_ptr<HardwareTracking> tracking
)
	: m_description(descriptor)
	, m_visualizer(std::move(visualizer))
	, m_bodygraph(std::move(bodygraph))
	, m_discoverer(std::move(discoverer))
	, m_playback(std::move(playback))
	, m_haptics(std::move(haptics))
	, m_originator(parentPlugin)
	, m_trackingProvider(std::move(tracking))
{
	m_discoverer->Discover();
	m_bodygraph->fetchDynamically();

	int a = 3;
//	auto imus = m_discoverer->GetNodesOfType(nsvr_node_concept_inertial_tracker);
//	for (auto imu : imus) {
	//	m_trackingProvider->BeginStreaming(NodeId<local>{imu});
	//}

}

Device::~Device()
{
	std::cout << "DEVICE DESTRUCTOR\n";

}

//void Device::DispatchEvent(uint64_t event_id, const NullSpaceIPC::SimpleHaptic & simple, const std::vector<nsvr_region>& regions)
//{
//	
//	auto haptic_only = m_discoverer->FilterByType(m_bodygraph->GetNodesAtRegions(regions), nsvr_node_concept_haptic);
//
//	//So I guess the question is.. what do we do here.
//	//Do we dispatch to only haptic nodes, in order of waveform->continueous->buffered?
//	//Do we dispatch to anything that supports the correct apis? 
//	//I think we need some Policies. 
//
//	//policy{
//
//	//if [haptic] and [supports waveform] then send IF  policy_strict?
//	//if [led] and [supports waveform] then send IF policy_relaxed?
//	//policy_greedy and policy_restrained?
//
//
//	//so when you send an event you can say "let this be a greedy event, it will try to play on whatever it possibly can.
//	//So if its a waveform, sure its gonna try to play on that kicker and that LED, not just an erm.
//	//Or, "let this be a restrained event, only play on specific hardware that is really meant to support it". 
//	
//	/*
//	Making up terminology: an event has an "execution class" such as haptic, led, kicker, etc.
//	These events are implemented, at this level, by using the low level apis that somewhat correspond to the events.
//
//	So with a "restrained" hint, it will only play on nodes that match the "execution class" of the event, and support the correct
//	apis. With a "relaxed" hint, it will play on nodes that don't necessarily match the execution class, but do support the correct
//	apis. 
//
//	What does this get us? Just a ton of complexity?
//	Freebees for the dev like: controlling all leds with haptics. Controlling a heat pad with a waveform.
//	Triggering a kicker with a normal preset waveform instead of a specific impact event. 
//
//	This could be the thing that allows the developer to say "hey, I want to target other hardware. I get that it might not be as good,
//	but use your intelligence and do it". Or the dev can say, "Hey I want to target only this specific hardware". 
//	
//	
//	*/
//	for (const auto& node : haptic_only) {
//		//if (m_discoverer->)
//		//m_haptics->SubmitSimpleHaptic(event_id, node, SimpleHaptic(simple.effect(), simple.duration(), simple.strength()));
//	}
//
//	m_playback->CreateEventRecord(event_id, haptic_only);
//}



DeviceId<local> Device::id() const
{
	return DeviceId<local>{m_description.id};
}


void Device::Deliver(uint64_t eventId, const NullSpaceIPC::LocationalEvent & event, const std::vector<nsvr_region>& regions)
{
	
	const auto& localNodes = m_bodygraph->GetNodesAtRegions(regions);
	//stupid transform because this isn't consistent yet
	std::vector<NodeId<local>> wrappedNodes;
	for (nsvr_node_id raw : localNodes) {
		wrappedNodes.emplace_back(raw);
	}
	Deliver(eventId, event, wrappedNodes);
}

void Device::Deliver(uint64_t eventId, const NullSpaceIPC::LocationalEvent &event, const std::vector<NodeId<local>>& nodes)
{
	switch (event.events_case()) {
	case NullSpaceIPC::LocationalEvent::kSimpleHaptic:
		handle(eventId, event.simple_haptic(), nodes);
		break;
	case NullSpaceIPC::LocationalEvent::kContinuousHaptic:
		handle(eventId, event.continuous_haptic(), nodes);
		break;
	default:
		BOOST_LOG_SEV(clogger::get(), nsvr_severity_warning) << "Unknown 'event' case: " << event.events_case();
	}
}

void Device::Deliver(uint64_t eventId, const NullSpaceIPC::PlaybackEvent &event)
{
	handle(eventId, event);
}

std::string Device::name() const
{
	return m_description.displayName;
}


nsvr_device_concept Device::concept() const
{
	return m_description.concept;
}

std::string Device::parentPlugin() const
{
	return m_originator;
}

void Device::OnReceiveTrackingUpdate(TrackingHandler handler)
{
	m_trackingProvider->OnUpdate([this, callback = handler](NodeId<local> node, nsvr_quaternion* quat) {
		const auto& regions = m_bodygraph->GetRegionsForNode(node.value);
		for (const auto& region : regions) {
			callback(region, quat);
		}
	});
}

void Device::ForEachNode(std::function<void(Node*)> action)
{
	m_discoverer->ForEachNode(action);
}

void Device::UpdateVisualizer(double dt)
{
	m_visualizer->simulate(dt);
}

std::vector<std::pair<nsvr_region, RenderedNode>> Device::RenderVisualizer()
{
	std::vector<std::pair<nsvr_region, RenderedNode>> taggedNodes;
	std::vector<RenderedNode> nodes = m_visualizer->render();
	for (const auto& node : nodes) {
		const auto& regions = m_bodygraph->GetRegionsForNode(node.Id.value);
		for (nsvr_region region : regions) {
			taggedNodes.emplace_back(region, node);
		}
	}
	return taggedNodes;
}

std::vector<nsvr_node_id> toRawNodeIds(const std::vector<NodeId<local>>& targetNodes) {

	std::vector<nsvr_node_id> nodes;
	nodes.reserve(targetNodes.size());

	for (NodeId<local> node : targetNodes) {
		nodes.push_back(node.value);
	}
	return nodes;
}

void Device::handle(uint64_t eventId, const NullSpaceIPC::ContinuousHaptic & event, const std::vector<NodeId<local>>& targetNodes)
{
	handle(eventId, event, toRawNodeIds(targetNodes));
}

void Device::handle(uint64_t eventId, const NullSpaceIPC::SimpleHaptic & event, const std::vector<NodeId<local>>& targetNodes)
{
	handle(eventId, event, toRawNodeIds(targetNodes));
}



void Device::handle(uint64_t eventId, const NullSpaceIPC::ContinuousHaptic& event, const std::vector<nsvr_node_id>& targetNodes)
{
	auto onlyHaptic = m_discoverer->FilterByType(targetNodes, nsvr_node_concept_haptic);
	for (nsvr_node_id hapticNode : onlyHaptic) {
		//todo FILL IN DATA
		m_haptics->Submit(eventId, hapticNode, ContinuousData{});
	}
	m_playback->CreateEventRecord(eventId, onlyHaptic);


}

void Device::handle(uint64_t eventId, const NullSpaceIPC::SimpleHaptic& event, const std::vector<nsvr_node_id>& targetNodes)
{
	auto onlyHaptic = m_discoverer->FilterByType(targetNodes, nsvr_node_concept_haptic);
	for (nsvr_node_id hapticNode : onlyHaptic) {
		//todo FILL IN DATA
		m_haptics->Submit(eventId, hapticNode, WaveformData{event.duration(), event.strength(), event.effect()});
	}
	m_playback->CreateEventRecord(eventId, onlyHaptic); 

}


void Device::handle(uint64_t id, const NullSpaceIPC::PlaybackEvent& playbackEvent)
{

	switch (playbackEvent.command()) {
	case NullSpaceIPC::PlaybackEvent_Command_UNPAUSE:
		m_playback->Resume(id);
		break;
	case NullSpaceIPC::PlaybackEvent_Command_PAUSE:
		m_playback->Pause(id);
		break;
	case NullSpaceIPC::PlaybackEvent_Command_CANCEL:
		m_playback->Cancel(id);
		break;
	default:
		break;
	}
}


