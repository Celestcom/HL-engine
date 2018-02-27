#include "stdafx.h"
#include "DeviceManager.h"
#include <iostream>
#include "Device.h "
#include "hardlight_device_version.h"
#include "AsyncPacketRequest.h"
static std::array<uint8_t, 16> version_packet = { 0x24,0x02,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x0D,0x0A };
static std::array<uint8_t, 16> uuid_packet = { 0x24,0x02,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x0D,0x0A };

void requestSuitVersion(boost::lockfree::spsc_queue<uint8_t>& output)
{
	output.push(version_packet.data(), version_packet.size());
}

void requestUuid(boost::lockfree::spsc_queue<uint8_t>& output)
{
	output.push(uuid_packet.data(), uuid_packet.size());
}

DeviceManager::DeviceManager(std::string path)
	: m_ioService()
	, m_path(path)
	, m_deviceIds()
	, m_doctor(m_ioService.GetIOService())
	, m_recognizer(m_ioService.GetIOService(), &m_doctor)
	, m_requestVersionTimeout(boost::posix_time::millisec(200))
	, m_requestVersionTimer(m_ioService.GetIOService())
	, m_idPool()
	, m_devicePollTimeout(boost::posix_time::millisec(5))
	, m_devicePollTimer(m_ioService.GetIOService())
	, m_deviceLock()
{
	

	m_recognizer.on_recognize([this](connection_info info) {
		//dispatch the event on our own IO service's thread (it came from the serial_connection_manager's thread)
		m_ioService.GetIOService().post([this, info]() { 
			handle_recognize(info);
		});
	});

	m_recognizer.on_unrecognize([this](std::string interface_name) {
		//dispatch the event on our own IO service's thread (it came from the serial_connection_manager's thread)
		m_ioService.GetIOService().post([this, interface_name]() {
			handle_unrecognize(interface_name);
		});
	});
	
	m_recognizer.start();


	m_devicePollTimer.expires_from_now(m_devicePollTimeout);
	m_devicePollTimer.async_wait([this](auto ec) { if (ec) { return; } 
		device_update();
	});
}

class connection_handler : public boost::static_visitor<std::unique_ptr<PotentialDevice>> {
public:
	connection_handler(boost::asio::io_service& io) : m_io(io){

	}
	std::unique_ptr<PotentialDevice> operator()(wired_connection conn) const {
		auto port = std::make_unique<boost::asio::serial_port>(m_io);

		boost::system::error_code ec;
		port->open(conn.port_name, ec);
		if (!port->is_open()) {
			core_log(nsvr_severity_error, "DeviceManager", "Unable to re-open the previously detected port " + conn.port_name);
			return {};
		}

		return std::make_unique<PotentialDevice>(std::move(port));
	}
	std::unique_ptr<PotentialDevice> operator()(wifi_connection conn) const {

		using tcp = boost::asio::ip::tcp;
		tcp::resolver resolver(m_io);
		tcp::resolver::query query(conn.host_name, conn.port_number);
		tcp::resolver::iterator iterator = resolver.resolve(query);

		auto socket = std::make_unique<tcp::socket>(m_io);

		return std::make_unique<PotentialDevice>(std::move(socket), iterator, conn.password);
	}
private:
	boost::asio::io_service& m_io;
};
void DeviceManager::handle_recognize(connection_info info)
{
	auto device = boost::apply_visitor(connection_handler(m_ioService.GetIOService()), info);
	if (!device) {
		return;
	}
	
	//begins reading and writing to port
	device->io->start();

	//begins synchronizing incoming packets from the device
	device->synchronizer->start();

	//in order for a device to move from "potentially found" to "actually present", we need it to return its version
	device->dispatcher->AddConsumer(inst::Id::GET_VERSION, [this, id = device->id](auto packet) {
		handle_connect(id, packet);
	});

	//some suits, or a zombie suit, may end up streaming tracking data on connect. We should tell the user to reset the suit if this happens.
	device->dispatcher->AddConsumer(inst::Id::GET_TRACK_DATA, [this, id = device->id](auto packet) {
		core_log(nsvr_severity_warning, "DeviceManager", "It seems that a device might be connected on interface "+ id + ", but can't confirm. Please power cycle the device.");
	});


	device->synchronizer->on_packet([weakDispatch = std::weak_ptr<PacketDispatcher>(device->dispatcher)](auto packet) {
		if (auto dispatcher = weakDispatch.lock()) {
			dispatcher->Dispatch(std::move(packet));
		}
	});

	requestSuitVersion(*device->io->outgoing_queue());
	requestSuitVersion(*device->io->outgoing_queue());
	requestUuid(*device->io->outgoing_queue());

	m_potentials.insert(std::make_pair(device->id, std::move(device)));
}

void DeviceManager::handle_unrecognize(std::string interface_name)
{
	std::lock_guard<std::mutex> guard(m_deviceLock);

	auto it = std::find_if(m_deviceIds.begin(), m_deviceIds.end(), [port = interface_name](const auto& kvp) { return kvp.second == port; });

	if (it != m_deviceIds.end()) {
		m_doctor.release_patient();

		nsvr_device_event_raise(m_core, nsvr_device_event_device_disconnected, it->first);
		m_idPool.Release(it->first);

		m_devices.erase(it->second);
		m_deviceIds.erase(it);
	}




}



class doctor_report_visitor : public boost::static_visitor<void> {
public:
	doctor_report_visitor() : status(0), error(0) {}
	void operator()(Doctor::Status status) {
		error = 0;
		this->status = static_cast<int>(status);
	}

	void operator()(HardwareFailures failure) {
		status = 0;
		error = static_cast<uint64_t>(failure);
	}

	void query(int* outstatus, uint64_t* outerror) {
		*outstatus = this->status;
		*outerror = this->error;
	}
private:
	int status;
	uint64_t error;
};
void DeviceManager::GetCurrentDeviceState(int * status, uint64_t* error)
{
	doctor_report_visitor results{};
	boost::apply_visitor(results, m_doctor.query_patient());
	results.query(status, error);
}

void DeviceManager::handle_connect(std::string portName, Packet versionPacket) {

	auto id = m_idPool.Request();
	m_deviceIds[id] = portName;
	{
		std::lock_guard<std::mutex> guard(m_deviceLock);

		if (m_potentials.find(portName) == m_potentials.end()) {
			return;
		}

		auto version = parse_version(versionPacket);
		
		auto potential = std::move(m_potentials.at(portName));

		m_potentials.erase(portName);
		

		potential->dispatcher->ClearConsumers();

		//put diagnostics code here

		m_doctor.accept_patient(id, potential->dispatcher.get(), potential->io->outgoing_queue().get());


		auto real = std::make_unique<Device>(m_ioService.GetIOService(), m_path, std::move(potential), version);
		real->Configure(m_core);


		m_devices.insert(std::make_pair(portName, std::move(real)));


	}

	


	nsvr_device_event_raise(m_core, nsvr_device_event_device_connected, id);


}

void DeviceManager::device_update()
{
	std::lock_guard<std::mutex> guard(m_deviceLock);

	float util_ratio = 0.0;
	for (auto& kvp : m_devices) {
		kvp.second->Update();
		util_ratio = std::max(util_ratio, kvp.second->GetIoUtilizationRatio());
	}

	m_devicePollTimeout = boost::posix_time::millisec((45 * std::pow(util_ratio, 2.71f)) + 10);

	m_devicePollTimer.expires_from_now(m_devicePollTimeout);
	m_devicePollTimer.async_wait([this](auto ec) { if (ec) { return; }
		device_update();
	});

}



DeviceManager::~DeviceManager()
{
	m_recognizer.stop();
	m_devicePollTimer.cancel();
}



int DeviceManager::configure(nsvr_core * core)
{
	global_core = core;
	m_core = core;

	nsvr_plugin_device_api device_api;
	device_api.client_data = this;
	device_api.enumeratenodes_handler = [](nsvr_device_id device_id, nsvr_node_ids* ids, void* cd) {
		AS_TYPE(DeviceManager, cd)->EnumerateNodesForDevice(device_id, ids);
	};
	device_api.enumeratedevices_handler = [](nsvr_device_ids* ids, void* cd) {
		AS_TYPE(DeviceManager, cd)->EnumerateDevices(ids);
	};
	device_api.getdeviceinfo_handler = [](nsvr_device_id id, nsvr_device_info* info, void* cd) {
		AS_TYPE(DeviceManager, cd)->GetDeviceInfo(id, info);
	};
	device_api.getnodeinfo_handler = [](nsvr_device_id did, nsvr_node_id id, nsvr_node_info* info, void* cd) {
		AS_TYPE(DeviceManager, cd)->GetNodeInfo(did, id, info);
	};
	nsvr_register_device_api(core, &device_api);

	nsvr_plugin_diagnostics_api diagnostics_api;
	diagnostics_api.client_data = this;
	diagnostics_api.updatemenu_handler = [](nsvr_diagnostics_ui* ui, void* cd) {
		AS_TYPE(DeviceManager, cd)->Render(ui);
	};

	nsvr_register_diagnostics_api(core, &diagnostics_api);



	nsvr_plugin_verification_api verification_api;
	verification_api.client_data = this;
	verification_api.getcurrentdevicestate_handler = [](int* outStatus, uint64_t* outError, void* cd) {
		AS_TYPE(DeviceManager, cd)->GetCurrentDeviceState(outStatus, outError);
	};
	nsvr_register_verification_api(core, &verification_api);
	return 1;
}

void DeviceManager::Render(nsvr_diagnostics_ui* ui) {
	std::lock_guard<std::mutex> guard(m_deviceLock);


	for (auto& device : m_devices) {
		device.second->Render(ui);
	}
}
void DeviceManager::EnumerateNodesForDevice(nsvr_device_id id, nsvr_node_ids * ids)
{
	std::lock_guard<std::mutex> guard(m_deviceLock);


	auto portName = m_deviceIds.at(id);
	m_devices.at(portName)->EnumerateNodesForDevice(ids);
}

void DeviceManager::EnumerateDevices(nsvr_device_ids * ids)
{
	std::lock_guard<std::mutex> guard(m_deviceLock);


	ids->device_count = m_deviceIds.size();

	std::size_t index = 0;
	for (const auto& id : m_deviceIds) {
		ids->ids[index++] = id.first;
	}
}

void DeviceManager::GetDeviceInfo(nsvr_device_id id, nsvr_device_info * info)
{
	std::lock_guard<std::mutex> guard(m_deviceLock);


	auto portName = m_deviceIds.at(id);
	m_devices.at(portName)->GetDeviceInfo(info);
}

void DeviceManager::GetNodeInfo(nsvr_device_id device_id, nsvr_node_id node_id, nsvr_node_info * info)
{
	std::lock_guard<std::mutex> guard(m_deviceLock);

	auto portName = m_deviceIds.at(device_id);
	m_devices.at(portName)->GetNodeInfo(node_id, info);
}
