#include "stdafx.h"
#include "PluginInstance.h"

#include <boost/type_index.hpp>
#include <iostream>
#include "IHardwareDevice.h"
#include "DeviceContainer.h"
PluginInstance::PluginInstance(boost::asio::io_service& io, std::string fileName, DeviceContainer& d) :
	m_fileName(fileName), 
	m_loaded{ false },
	m_pluginFunctions{},
	m_pluginRegisterFunction{},
	m_apis(),
	m_eventHandler(io),
	m_facade(m_apis, m_eventHandler),
	m_deviceContainer(d)
	
{
	m_eventHandler.Subscribe(nsvr_device_event_device_connected, [this](uint64_t device_id) {
		m_deviceContainer.AddDevice(device_id, m_apis, m_eventHandler);
	});

	m_eventHandler.Subscribe(nsvr_device_event_device_disconnected, [this](uint64_t device_id) {
		m_deviceContainer.RemoveDevice(device_id);
	});

}


PluginInstance::~PluginInstance()
{
}

bool PluginInstance::ParseManifest()
{
	std::string manifestFilename = m_fileName + "_manifest.json";
	if (!DriverConfigParser::IsValidConfig(manifestFilename)) {
		std::cout << manifestFilename << " is not a valid config file\n";
		return false;
	}

	try {
		m_descriptor = DriverConfigParser::ParseConfig(manifestFilename);

		
	}
	catch (const std::exception&) {
		std::cout << "Unable to parse descriptor for " << m_fileName << '\n';
		return false;
	}

	return true;

}



//Precondition: linked successfully
//does nothing if already loaded. May want to reinstantiate instead. We'll see.
bool PluginInstance::Load()
{
	if (m_loaded) {
		return true;
	}


	if (m_pluginRegisterFunction) {
		m_pluginRegisterFunction(&m_pluginFunctions);

		m_loaded = m_pluginFunctions.init(&m_pluginPointer);
		
	}	
	
	return m_loaded;
	
	
}



//precondition: successfully loaded
bool PluginInstance::Configure()
{
	if (m_pluginFunctions.configure) {
		return m_pluginFunctions.configure(m_pluginPointer, reinterpret_cast<nsvr_core*>(&m_facade));
	}
	
	return false;
}

bool PluginInstance::run_update_loop_once(uint64_t dt)
{
	if (auto api = m_apis.GetApi<updateloop_api>()) {
		try {
			api->submit_update(dt);
		}
		catch (const std::runtime_error& err) {
			BOOST_LOG_TRIVIAL(error) << "Runtime ERROR in plugin " << m_displayName << ": " << err.what();
			return false;
		}
	}

	return true;
}

bool PluginInstance::Link()
{
	boost::system::error_code loadFailure;

	m_dll = std::make_unique<boost::dll::shared_library>(m_fileName, boost::dll::load_mode::append_decorations, loadFailure);
	
	if (loadFailure) {
		std::cout << "Failed to load " << m_fileName << ": " << loadFailure.message() << ".\n";
		return false;
	}

	std::cout << "Loaded " << m_fileName << ".\n";

	if (!tryLoad(m_dll, "nsvr_plugin_register", m_pluginRegisterFunction)) {
		return false;
	}
	
	return true;

}

//precondition: linked successfully
bool PluginInstance::Unload()
{
	bool result = false;
	if (m_pluginFunctions.free) {
		result = m_pluginFunctions.free(m_pluginPointer);
		m_pluginPointer = nullptr;
	}
	m_dll.reset();

	m_loaded = false;
	return result;
}

bool PluginInstance::IsLoaded() const
{
	return m_loaded;
}

std::string PluginInstance::GetFileName() const
{
	return m_fileName;
}

std::string PluginInstance::GetDisplayName() const
{
	return m_displayName;
}


