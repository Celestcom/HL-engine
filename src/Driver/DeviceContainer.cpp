#include "stdafx.h"
#include "DeviceContainer.h"

#include <experimental/vector>
#include "HardwareBodygraphCreator.h"
#include "HardwareNodeEnumerator.h"
#include "HardwarePlaybackController.h"
#include "HardwareHapticInterface.h"
#include "HardwareTracking.h"
#include "HapticInterface.h"
#include "DriverMessenger.h"
#include "DeviceIds.h"
#include "DeviceVisualizer.h"

#include "DeviceBuilder.h"

#include "logger.h"
DeviceContainer::DeviceContainer()

	: m_onDeviceAdded()
	, m_onDeviceRemoved()
	, m_devices()
	, m_deviceAddedSubs()
	, m_deviceRemovedSubs()
	, m_deviceLock()
{
}

void DeviceContainer::AddDevice(nsvr_device_id id, PluginApis & apis, Parsing::BodyGraphDescriptor bodyGraphDescriptor, std::string originatingPlugin, PluginInstance::DeviceResourceBundle& resources)
{
	/*if (auto api = apis.GetApi<device_api>()) {

		nsvr_device_ids ids = { 0 };

		api->submit_enumeratedevices(&ids);

		for (std::size_t i = 0; i < ids.device_count; i++) {

			nsvr_device_info info = { 0 };
			api->submit_getdeviceinfo(ids.ids[i], &info);

			DeviceDescriptor desc;
			desc.displayName = std::string(info.name);
			desc.id = info.id;
			desc.concept = info.concept;
			addDevice(desc, apis, std::move(bodyGraphDescriptor), originatingPlugin, resources);
		}
	}*/
	DeviceDescriptor descriptor;
	if (resources->descriptor) {
		descriptor = *resources->descriptor;

	}
	else {
		nsvr_device_info info = { 0 };
		apis.GetApi<device_api>()->submit_getdeviceinfo(id, &info);

		DeviceDescriptor desc;
		desc.displayName = std::string(info.name);
		desc.id = info.id;
		desc.concept = info.concept;
		descriptor = desc;
	}

	auto builder = DeviceBuilder(&apis, resources, id)
		.WithDescriptor(descriptor)
.WithBodygraphDescriptor(bodyGraphDescriptor)
			.WithOriginatingPlugin(originatingPlugin)
		
		.Build();


			m_deviceLock.lock();
			m_devices.push_back(std::move(builder));
			Device* newlyAdded = m_devices.back().get();
			m_deviceLock.unlock();

			m_onDeviceAdded(newlyAdded);
}










void DeviceContainer::RemoveDevice(DeviceId<local> id, std::string pluginName)
{
	m_deviceLock.lock();

	auto it = std::find_if(m_devices.begin(), m_devices.end(), [id, pluginName](const auto& device) { return device->id() == id && device->parentPlugin() == pluginName; });
	if (it != m_devices.end()) {
		m_onDeviceRemoved(it->get());
		m_devices.erase(it);
	}

	m_deviceLock.unlock();


}


void DeviceContainer::EachDevice(DeviceFn forEach)
{
	std::lock_guard<std::mutex> guard(m_deviceLock);
	for (auto& ptr : m_devices) {
		forEach(ptr.get());
	}
}



Device* DeviceContainer::Get(std::string plugin, DeviceId<local> id)
{
	std::lock_guard<std::mutex> guard(m_deviceLock);

	auto it = std::find_if(m_devices.begin(), m_devices.end(), [id, plugin](const auto& device) { return device->id() == id && device->parentPlugin() == plugin; });
	if (it != m_devices.end()) {
		return (*it).get();
	}
	else {
		return nullptr;
	}
}

void DeviceContainer::OnDeviceAdded(DeviceEvent::slot_type slot)
{
	m_onDeviceAdded.connect(slot);
}

void DeviceContainer::OnDeviceRemoved(DeviceEvent::slot_type slot)
{
	m_onDeviceRemoved.connect(slot);
}


