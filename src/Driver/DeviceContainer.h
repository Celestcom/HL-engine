#pragma once
#include <memory>
#include "IHardwareDevice.h"
#include <unordered_map>

#include <mutex>
// Should break this into two interfaces: adding/removing, and the data operations
class DeviceContainer {
public:
	using DeviceFn = std::function<void(Device*)>;
	void AddDevice(const DeviceDescriptor&, PluginApis&, PluginEventSource&);
	void AddDevice(uint64_t id, PluginApis& apis, PluginEventSource& ev);
	void RemoveDevice(const std::string&);
	
	void RemoveDevice(uint64_t id);
	void Each(DeviceFn);

	DeviceContainer();


	void OnSystemAdded(DeviceFn);
	void OnPreSystemRemoved(DeviceFn);

private:

	std::vector<std::unique_ptr<Device>> m_devices;

	std::vector<DeviceFn> m_deviceAddedSubs;
	std::vector<DeviceFn> m_deviceRemovedSubs;

	std::mutex m_deviceLock;
	void notify(const std::vector<DeviceFn>& devices, Device* device);
	
};


