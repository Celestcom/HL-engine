#include "stdafx.h"
#include "HardlightPlugin.h"

#include "IoService.h"
#include "Heartbeat.h"
#include "Synchronizer.h"

#include <typeinfo>
#include "PluginAPIWrapper.h"
#include "IMU_ID.h"
#include "DeviceManager.h"

#include "synchronizer2.h"

nsvr_core* global_core = nullptr;
//note: can make firmware unique
HardlightPlugin::HardlightPlugin(boost::asio::io_service& io, const std::string& data_dir, std::unique_ptr<PotentialDevice> device, hardlight_device_version version) :
	m_core{ nullptr },
	m_io(io),
	m_hwIO(std::move(device->io)),
	m_dispatcher(std::move(device->dispatcher)),
	m_firmware(std::make_shared<FirmwareInterface>(data_dir, device->io->outgoing_queue(), m_io)),
	m_monitor(std::make_shared<Heartbeat>(m_io, m_firmware)),
	m_synchronizer(device->synchronizer),
	m_device(),
	m_imus(*m_dispatcher),
	m_version(version)

{
	


	
	

	m_dispatcher->AddConsumer(PacketType::Ping, [this](const auto&) { m_monitor->ReceiveResponse(); });
	m_dispatcher->AddConsumer(PacketType::ImuData, [this](const auto&) { m_monitor->ReceiveResponse(); });
	



	
	m_imus.AssignMapping(0x3c, Imu::Chest, NODE_IMU_CHEST); 
	m_imus.AssignMapping(0x39, Imu::Right_Upper_Arm, NODE_IMU_RIGHT_UPPER_ARM);
	m_imus.AssignMapping(0x3a, Imu::Left_Upper_Arm, NODE_IMU_LEFT_UPPER_ARM);

	



}

HardlightPlugin::~HardlightPlugin()
{
	m_imus.stop();
	m_synchronizer->stop();
	m_hwIO->stop();

}




struct bodygraph_region {
	nsvr_bodygraph_region* region;

	bodygraph_region() {
		nsvr_bodygraph_region_create(&region);
	}

	bodygraph_region& setLocation(nsvr_bodypart bodypart, double ratio, double rot) {
		nsvr_bodygraph_region_setlocation(region, bodypart, ratio, rot);
		return *this;
	}
	bodygraph_region& setDimensions(double width_cm, double height_cm) {
		nsvr_bodygraph_region_setboundingboxdimensions(region, width_cm, height_cm);
		return *this;
	}
	~bodygraph_region() {
		nsvr_bodygraph_region_destroy(&region);
		assert(region == nullptr);
	}
};

int HardlightPlugin::Configure(nsvr_core* core)
{

	


	m_core = core;
	global_core = core;
	m_device.Configure(core);

	nsvr_plugin_tracking_api tracking_api;
	tracking_api.beginstreaming_handler = [](nsvr_tracking_stream* stream, nsvr_node_id region, void* client_data) {
		AS_TYPE(HardlightPlugin, client_data)->BeginTracking(stream, region);
	};
	tracking_api.endstreaming_handler = [](nsvr_node_id region, void* client_data) {
		AS_TYPE(HardlightPlugin, client_data)->EndTracking(region);
	};
	tracking_api.client_data = this;
	nsvr_register_tracking_api(core, &tracking_api);
	

	nsvr_plugin_bodygraph_api body_api;
	body_api.setup_handler = [](nsvr_bodygraph* g, void* cd) {
		AS_TYPE(HardlightPlugin, cd)->SetupBodygraph(g);
	
	};
	body_api.client_data = this;
	nsvr_register_bodygraph_api(core, &body_api);


	
	
	return 1;
}

void HardlightPlugin::BeginTracking(nsvr_tracking_stream* stream, nsvr_node_id id)
{
	m_imus.AssignStream(stream, id);
	m_firmware->EnableTracking();
}

void HardlightPlugin::EndTracking(nsvr_node_id id)
{
	m_imus.RemoveStream(id);
	m_firmware->DisableTracking();
}

void HardlightPlugin::EnumerateNodesForDevice(nsvr_node_ids * ids)
{
	m_device.EnumerateNodesForDevice(ids);
}

void HardlightPlugin::EnumerateDevices(nsvr_device_ids* ids)
{
	if (m_monitor->IsConnected()) {
		ids->device_count = 1;
		ids->ids[0] = 0;
	}
	else {
		ids->device_count = 0;
	}
}

void HardlightPlugin::GetDeviceInfo(nsvr_device_info * info)
{
	static std::unordered_map<int, std::string> products = {{2, "MarkII"}, {3, "MarkIII" }};

	std::string mark = "(Unknown Version)";
	if (products.find(m_version.product) != products.end()) {
		mark = products.at(m_version.product);
	}

	std::string device_name("Hardlight Suit " + mark);
	std::copy(device_name.begin(), device_name.end(), info->name);
	info->concept = nsvr_device_concept_suit;
	
}

void HardlightPlugin::GetNodeInfo(nsvr_node_id id, nsvr_node_info* info)
{

	m_device.GetNodeInfo(id, info);
}

void HardlightPlugin::SetupBodygraph(nsvr_bodygraph * g)
{

	m_device.SetupDeviceAssociations(g);
	
}
std::string stringifyStatusBits(HL_Unit status) {
	std::stringstream ss;
	for (auto val : HL_Unit::_values()) {
		if (status & val) {
			ss << val._to_string() << "|";
		}
	}
	return ss.str();
}

void HardlightPlugin::Render(nsvr_diagnostics_ui * ui)
{
	static const std::vector<std::string> syncStates = {
		"Synchronized",
		"SearchingForSync",
		"ConfirmingSync",
		"ConfirmingSyncLoss"
	};
	
	ui->keyval("Product version", std::to_string(m_version.product).c_str());
	ui->keyval("Product revision", std::to_string(m_version.revision).c_str());
	ui->keyval("Firmware major", std::to_string(m_version.firmware_a).c_str());
	ui->keyval("Firmware minor", std::to_string(m_version.firmware_b).c_str());


	ui->keyval("Synchronizer state", syncStates[(int)m_synchronizer->state()].c_str());
	
	
	if (ui->button("TRACKING_ENABLE")) {
		m_firmware->EnableTracking();
	}
	if (ui->button("TRACKING_DISABLE")) {
		m_firmware->DisableTracking();
	}

	if (ui->button("GET_TRACK_STATUS")) {
		m_firmware->RequestTrackingStatus();
	}
	

	static FirmwareInterface::AudioOptions opts{ 0x00, 0x01, 0x40, 0x00, 0x7F };

	if (ui->slider_int("VibeCtrl", &opts.VibeCtrl, 0, 5)	||
		ui->slider_int("AudioMin", &opts.AudioMin, 0, 255)	||
		ui->slider_int("AudioMax", &opts.AudioMax, 0, 255)	||
		ui->slider_int("MinDrv", &opts.MinDrv, 0, 125)		||
		ui->slider_int("MaxDrv", &opts.MaxDrv, 0, 125))	
	{
		for (int i = static_cast<int>(Location::Lower_Ab_Right); i < static_cast<int>(Location::Error); i++) {
			m_firmware->EnableAudioMode(static_cast<Location>(i), opts);
		}
	}
	

	if (ui->button("Disable audio on all")) {
		for (int i = static_cast<int>(Location::Lower_Ab_Right); i < static_cast<int>(Location::Error); i++) {
			m_firmware->EnableIntrigMode(static_cast<Location>(i));
		}
	}

	static int rtpVol = 0;
	if (ui->button("Enable RTP Mode on all")) {
		for (int i = static_cast<int>(Location::Lower_Ab_Right); i < static_cast<int>(Location::Error); i++) {
			m_firmware->EnableRtpMode(static_cast<Location>(i));
		}
	}
	if (ui->slider_int("RTP", &rtpVol, 0, 127)) {
		for (int i = static_cast<int>(Location::Lower_Ab_Right); i < static_cast<int>(Location::Error); i++) {
			m_firmware->PlayRtp(static_cast<Location>(i), rtpVol);
		}
	}

	auto imuInfo = m_imus.GetInfo();
	for (const auto& imu : imuInfo) {
		std::string imuId("Imu " + std::to_string((int)imu.firmwareId));
		std::string friendlyId("(friendly = " + std::to_string((int)imu.friendlyName) + ")");
		ui->keyval(imuId.c_str(), friendlyId.c_str());
		ui->keyval("status", stringifyStatusBits(imu.status).c_str());
	}


	ui->keyval("Total bytes sent", std::to_string(m_hwIO->bytes_written()).c_str());
	ui->keyval("Total bytes rec'd", std::to_string(m_hwIO->bytes_read()).c_str());
	
}

void HardlightPlugin::PollEvents()
{
	constexpr auto ms_fraction_of_second = (1.0f / 1000.f);
	auto dt = 5 * ms_fraction_of_second;


	auto commands = m_device.GenerateHardwareCommands(dt);
	m_firmware->Execute(commands);
}

