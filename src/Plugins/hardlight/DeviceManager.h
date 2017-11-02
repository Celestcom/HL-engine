#pragma once

#include "serial/hardware_device_recognizer.h"

#include "BoostSerialAdapter.h"
#include "Synchronizer.h"
#include "PacketDispatcher.h"
#include "IoService.h"
#include "Heartbeat.h"
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "synchronizer2.h"

#include "IdPool.h"
class HardlightPlugin;
struct PotentialDevice {
	std::unique_ptr<BoostSerialAdapter> adapter;
	std::unique_ptr<PacketDispatcher> dispatcher;

	std::shared_ptr<synchronizer2> synchronizer;

	PotentialDevice(boost::asio::io_service& io) {
		dispatcher = std::make_unique<PacketDispatcher>();
		adapter = std::make_unique<BoostSerialAdapter>(io);
		synchronizer = std::make_shared<synchronizer2>(io, adapter->GetDataStream());
	}
};
class DeviceManager {
public:
	DeviceManager(std::string path);
	~DeviceManager();

	int configure(nsvr_core* core);

	void EnumerateNodesForDevice(nsvr_device_id, nsvr_node_ids* ids);
	void EnumerateDevices(nsvr_device_ids* ids);
	void GetDeviceInfo(nsvr_device_id id, nsvr_device_info* info);
	void GetNodeInfo(nsvr_device_id device_id, nsvr_node_id id, nsvr_node_info* info);
	void Render(nsvr_diagnostics_ui* ui);
private:
	

	void handle_connect(std::string portName, Packet packet);
	void device_update();
	nsvr_core* m_core;
	IoService m_ioService;
	std::string m_path;
	hardware_device_recognizer m_recognizer;
	//map portname -> product

	std::unordered_map<std::size_t, std::string> m_deviceIds;
	std::unordered_map<std::string, std::unique_ptr<HardlightPlugin>> m_devices;
	std::unordered_map<std::string, std::unique_ptr<PotentialDevice>> m_potentials;

	boost::posix_time::milliseconds m_requestVersionTimeout;
	boost::asio::deadline_timer m_requestVersionTimer;

	boost::posix_time::millisec m_devicePollTimeout;
	boost::asio::deadline_timer m_devicePollTimer;

	IdPool m_idPool;

	void send_version_requests();
};