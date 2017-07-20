#pragma once
#include <type_traits>

#include "HardwareDataModel.h"
#include "cevent_internal.h"

#include "protobuff_defs/HighLevelEvent.pb.h"
#include <unordered_map>
class DriverMessenger;
class EventDispatcher;
class HardwareCoordinator
{
public:
	HardwareCoordinator(DriverMessenger& messenger, EventDispatcher& dispatcher);
	~HardwareCoordinator();

	void Register(nsvr_request_type type, nsvr_plugin_request_api::nsvr_request_handler handler, unsigned int, void* user_data);
	HardwareDataModel& Get(const std::string& name);

	void dispatch_event(nsvr::cevents::request_base& event);



	void HardwareCoordinator::Playback(uint32_t command, nsvr_playback_handle* existingHandle);

private:
	
	DriverMessenger& m_messenger;
	std::unordered_map<std::string, HardwareDataModel> m_hardware;

	struct user_event_handler {
		nsvr_plugin_request_api::nsvr_request_handler invoke;
		void* user_data;
		unsigned int target_version;
	};
	std::unordered_map<nsvr_request_type, std::vector<user_event_handler>> m_handlers;
	void updateTrackingForMessenger(const std::string& region, nsvr_quaternion quat);

	std::unordered_map<uint64_t, nsvr_playback_handle> m_activeEffects;
	void Dispatch(const NullSpaceIPC::SimpleHaptic& event, nsvr_playback_handle* handle);
};



