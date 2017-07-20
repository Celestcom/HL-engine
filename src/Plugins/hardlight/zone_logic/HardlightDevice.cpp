#include "stdafx.h"
#include "HardlightDevice.h"
#include <boost/uuid/random_generator.hpp>
#include <experimental/vector>
#include "PluginAPI.h"
#include "Locator.h"



HardlightDevice::HardlightDevice() 
{
	auto& translator = Locator::Translator();
	for (int loc = (int)Location::Lower_Ab_Right; loc != (int)Location::Error; loc++) {
		std::string locstring = translator.ToRegionFromLocation(Location(loc));

		//I'm just going to go ahead and memory leak these for now. 
		//Must wrap the API in c++ objects.
		nsvr_node* newNode;
		nsvr_node_create(&newNode);
		nsvr_node_setdisplayname(newNode, locstring.c_str());

		m_drivers.insert(std::make_pair(
			locstring, 
			std::make_unique<Hardlight_Mk3_ZoneDriver>(Location(loc), newNode))
		);

	}
}




void HardlightDevice::RegisterDrivers(nsvr_core* ctx)
{
	nsvr_plugin_request_api request_api;
	request_api.client_data = this;
	request_api.request_type = nsvr_request_type_lasting_haptic;
	request_api.request_handler = [](nsvr_request* req, void* client_data) {
		AS_TYPE(HardlightDevice, client_data)->handle(req);
	};
	nsvr_register_request_api(ctx, &request_api);


	nsvr_plugin_playback_api playback_api;
	playback_api.client_data = this;
	playback_api.pause_handler = [](nsvr_playback_handle* handle, void* client_data) {
		AS_TYPE(HardlightDevice, client_data)->Pause(handle);
	};
	playback_api.cancel_handler = [](nsvr_playback_handle* handle, void* client_data) {
		AS_TYPE(HardlightDevice, client_data)->Cancel(handle);
	};
	playback_api.unpause_handler = [](nsvr_playback_handle* handle, void* client_data) {
		AS_TYPE(HardlightDevice, client_data)->Unpause(handle);
	};

	nsvr_register_playback_api(ctx, &playback_api);

	
}

void HardlightDevice::handle( nsvr_request * event)
{
	nsvr_request_type type;
	nsvr_request_gettype(event, &type);
	switch (type) {
	case nsvr_request_type_lasting_haptic:
		executeLasting(event);
		break;
	default:
		break;
	}


}

void HardlightDevice::Pause(nsvr_playback_handle * handle)
{
	for (auto& driver : m_drivers) {
		driver.second->controlEffect(handle, 1);
	}
}

void HardlightDevice::Cancel(nsvr_playback_handle * handle)
{
	for (auto& driver : m_drivers) {
		driver.second->controlEffect(handle, 3);
	}
}

void HardlightDevice::Unpause(nsvr_playback_handle * handle)
{
	for (auto& driver : m_drivers) {
		driver.second->controlEffect(handle, 2);
	}
}

void HardlightDevice::executeBrief(nsvr_request * event)
{
	

	/*BasicHapticEventData data = {};
	data.duration = 0.0f;
	nsvr_request_briefhaptic_geteffect(event, &data.effect);
	nsvr_request_briefhaptic_getstrength(event, &data.strength);

	char region[32];
	nsvr_request_briefhaptic_getregion(event, region);

	if (m_drivers.find(region) != m_drivers.end()) {
		m_drivers.at(region)->consumeBrief(std::move(data));
	}*/
}

void HardlightDevice::executeLasting(nsvr_request * event)
{
	BasicHapticEventData data = {};
	nsvr_request_lastinghaptic_getduration(event, &data.duration);
	nsvr_request_lastinghaptic_geteffect(event, &data.effect);
	nsvr_request_lastinghaptic_getstrength(event, &data.strength);
	nsvr_playback_handle* handle;
	nsvr_request_gethandle(event, &handle);

	char region[32];
	nsvr_request_lastinghaptic_getregion(event, region);

	if (m_drivers.find(region) != m_drivers.end()) {
		m_drivers.at(region)->consumeLasting(std::move(data), handle);
	}

}


CommandBuffer HardlightDevice::GenerateHardwareCommands(float dt)
{
	CommandBuffer result;
	for (auto& driver_pair : m_drivers) {
		CommandBuffer cl = driver_pair.second->update(dt);
		result.insert(result.begin(), cl.begin(), cl.end());
	}
	return result;
		
}



//DisplayResults HardlightDevice::QueryDrivers()
//{
//	DisplayResults representations;
//
//	for (const auto& driver : m_drivers) {
//		auto result = driver->QueryCurrentlyPlaying();
//		if (result) {
//			representations.push_back(*result);
//		}
//	}
//
//	return representations;
//}
//


ZoneModel::UserCommand::UserCommand(): id(), command(Command::Unknown)
{
}

ZoneModel::UserCommand::UserCommand(ParentId id, Command c) : id(id), command(c)
{
}


