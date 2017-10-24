#pragma once
#include <cstdint>
#include <memory>

#include "PluginAPI.h"
#include "PacketDispatcher.h"
#include "FirmwareInterface.h"
#include "ImuConsumer.h"
#include "zone_logic/hardlightdevice.h"
#include "ScheduledEvent.h"


class IoService;
class BoostSerialAdapter;
class Heartbeat;
class Synchronizer;

class HardlightPlugin {
public:
	HardlightPlugin(const std::string& data_dir);
	~HardlightPlugin();

	int Configure(nsvr_core* ctx);
	void BeginTracking(nsvr_tracking_stream* stream, nsvr_node_id region);
	void EndTracking(nsvr_node_id region);
	void EnumerateNodesForDevice(nsvr_device_id, nsvr_node_ids* ids);
	void EnumerateDevices(nsvr_device_ids* ids);
	void GetDeviceInfo(nsvr_device_id id, nsvr_device_info* info);
	void GetNodeInfo(nsvr_node_id id, nsvr_node_info* info);
	void SetupBodygraph(nsvr_bodygraph* graph);

	void Render(nsvr_diagnostics_ui* ui);
private:
	nsvr_core* m_core;


	std::shared_ptr<IoService> m_io;
	PacketDispatcher m_dispatcher;
	std::unique_ptr<BoostSerialAdapter> m_adapter;
	FirmwareInterface m_firmware;

	HardlightDevice m_device;
	std::shared_ptr<Heartbeat> m_monitor;

	std::unique_ptr<Synchronizer> m_synchronizer;

	ScheduledEvent m_eventPull;
	bool m_running;

	ImuConsumer m_imus;



	

};