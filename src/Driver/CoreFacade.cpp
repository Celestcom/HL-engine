#include "stdafx.h"
#include "CoreFacade.h"
#include "PluginApis.h"
#include "PluginEventSource.h"


CoreFacade::CoreFacade(PluginApis& capabilities, PluginEventSource& eventHandler)
	: m_eventHandler(eventHandler)
	, m_pluginCapabilities(capabilities)
{

}



void CoreFacade::RaisePluginEvent(nsvr_device_event_type type, uint64_t id)
{
	m_eventHandler.Raise(type, id);
}
