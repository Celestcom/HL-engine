#pragma once
#include <cstdint>
#include <memory>

#include "PluginAPI.h"
#include "PacketDispatcher.h"
#include "FirmwareInterface.h"
#include "ImuConsumer.h"
#include "zone_logic/hardlightdevice.h"
#include "ScheduledEvent.h"
#include "HardwareIO.h"
#include "hardlight_device_version.h"

#include "HL_Firmware_Defines.h"

class synchronizer2;
struct PotentialDevice;
class BoostSerialAdapter;
class Heartbeat;
class Synchronizer;


struct MotorStatus {
	uint8_t MotorId;
	HL_Unit Status;
	MotorStatus(uint8_t id, HL_Unit status) : MotorId(id), Status(status) {}
	MotorStatus() : MotorId(0), Status(HL_Unit::_enumerated::None) {}
};
class Device {
public:
	Device(boost::asio::io_service& io, const std::string& data_dir, std::unique_ptr<PotentialDevice> device, hardlight_device_version version);
	~Device();

	int Configure(nsvr_core* ctx);
	void BeginTracking(nsvr_tracking_stream* stream, nsvr_node_id region);
	void EndTracking(nsvr_node_id region);
	void EnumerateNodesForDevice(nsvr_node_ids* ids);
	void EnumerateDevices(nsvr_device_ids* ids);
	void GetDeviceInfo(nsvr_device_info* info);
	void GetNodeInfo(nsvr_node_id id, nsvr_node_info* info);
	void SetupBodygraph(nsvr_bodygraph* graph);

	std::vector<MotorStatus> GetMotorStatus() const;

	void Render(nsvr_diagnostics_ui* ui);

	void Update();
private:

	void EnableAudio(nsvr_node_id id);
	void DisableAudio(nsvr_node_id id);

	nsvr_core* m_core;


	boost::asio::io_service& m_io;

	std::unique_ptr<HardwareIO> m_hwIO;

	std::shared_ptr<FirmwareInterface> m_firmware;
	HardlightDevice m_device;
	std::shared_ptr<Heartbeat> m_monitor;

	std::shared_ptr<synchronizer2> m_synchronizer;

	std::shared_ptr<PacketDispatcher> m_dispatcher;


	bool m_running;

	ImuConsumer m_imus;

	hardlight_device_version m_version;

	std::unordered_map<uint8_t, MotorStatus> m_motors;


};