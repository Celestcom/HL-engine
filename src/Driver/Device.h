#pragma once
#include <memory>
#include <string>
#include "NodeDiscoverer.h"
#include "BodyGraphCreator.h"
#include "PlaybackController.h"
#include "DeviceDescriptor.h"
#include "HapticInterface.h"
#include "DeviceIds.h"
//theory:
//NodeDiscoverer can be mocked out. So we can provide a bunch of fake nodes
//I think PlaybackController can be a concrete class and can get rid of the interface.

// BodyGraphCreator can be mocked out, but must be shared between the real device and a simulated device either way, so the simulation knows
//which nodes belong in which regions. 
//Then if we want to create an entirely fake device with no hardware present,
//we basically need to build out a skeleton of PluginApis that just has stubs for most of the apis, but e.g. returns the desired device name

// Then the function calls into the underlying apis will do nothing, but will also be replicated through the "instrumentation" pointer into simulated
// functionality. 

namespace NullSpaceIPC {
	class HighLevelEvent;
	class SimpleHaptic;
	class PlaybackEvent;
}
class Device {
public:
	Device(
		std::string parentPlugin,
		DeviceDescriptor descriptor, 
		std::shared_ptr<BodyGraphCreator>,
		std::unique_ptr<NodeDiscoverer>, 
		std::unique_ptr<PlaybackController>, 
		std::unique_ptr<HapticInterface>
	);
	void DispatchEvent(const NullSpaceIPC::HighLevelEvent& event);
	DeviceId<local> id() const;
	std::string name() const;
	nsvr_device_concept concept() const;
	std::string parentPlugin() const;

	void ForEachNode(std::function<void(Node*)> action);
private:
	std::string m_originator;
	DeviceDescriptor m_description;
	std::shared_ptr<BodyGraphCreator> m_bodygraph;
	std::unique_ptr<NodeDiscoverer> m_discoverer;
	std::unique_ptr<PlaybackController> m_playback;
	std::unique_ptr<HapticInterface> m_haptics;
	void handleSimpleHaptic(uint64_t event_id, const NullSpaceIPC::SimpleHaptic& simple);
	void handlePlaybackEvent(uint64_t event_id, const NullSpaceIPC::PlaybackEvent& playbackEvent);
};