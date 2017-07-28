#include "stdafx.h"
#include "IHardwareDevice.h"
#include "PluginInstance.h"


std::unique_ptr<Device> device_factories::createDevice(const HardwareDescriptor& description, PluginCapabilities& c, PluginEventHandler& e)
{
	if (description.concept == HardwareDescriptor::Concept::Suit) {
		return std::unique_ptr<Device>(new SuitDevice(description,c,e ));
	} 

	return nullptr;
}

SuitDevice::SuitDevice(const HardwareDescriptor & d, PluginCapabilities& c, PluginEventHandler& e)
	: m_associatedPlugin(c), m_descriptor(d)
{
	
}

bool SuitDevice::supportsRegion(const std::string & region) const
{
	for (const auto& node : m_descriptor.nodes) {
		if (region == node.region) {
			return true;
		}
	}
	return false;
}

void SuitDevice::doRequest(const nsvr::cevents::request_base& req)
{

	//we need to emulate the apis that arent' supported. So.
	request_api* api = m_associatedPlugin.GetApi<request_api>("request");
	if (api) {
		api->submit_request.invoke(
			reinterpret_cast<nsvr_request*>(
				const_cast<nsvr::cevents::request_base*>(&req)
			)
		);
	}
	
}

void SuitDevice::controlPlayback(uint64_t id, uint32_t command)
{
	playback_api* api = m_associatedPlugin.GetApi<playback_api>("playback");
	if (api) {
		switch (command) {
		case 1:
			api->submit_pause.invoke(id);
			break;
		case 2:
			api->submit_unpause.invoke(id);
			break;
		case 3:
			api->submit_cancel.invoke(id);
			break;
		default:
			BOOST_LOG_TRIVIAL(trace) << "[SuitDevice] Encountered a malformed playback request, command = " << command;
			break;
		}
	}
}




