#include "stdafx.h"
#include "Driver.h"
#include "IoService.h"
#include "ClientMessenger.h" 
#include "IntermediateHapticFormats.h"
#include "Encoder.h"
#include <functional>
#include "DriverCommand.pb.h"
#include "SuitVersionInfo.h"
#include <boost/log/core.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/trivial.hpp>
#include "MyTestLog.h"

#include "FirmwareInterface.h"
#include "IntermediateHapticFormats.h"


void extractDrvData(const packet& packet) {
	//as status register:
	uint8_t whichDrv = packet.raw[4];
	uint8_t whichReg = packet.raw[5];
	uint8_t data = packet.raw[3];

	bool over_current = data & 0x01;
	bool over_temperature = data & 0x02;
	
	bool mode = data & (0x07);

	bool standby = data & 0b00100000;



	BOOST_LOG_TRIVIAL(info) << "[DriverMain] DRVDIAG " << int(whichDrv) << "," <<int(over_current) << "," << int(over_temperature);

	
}

Driver::Driver() :
	_io(new IoService()),
	m_hardware(_io),
	m_messenger(_io->GetIOService()),
	m_statusPush(_io->GetIOService(), boost::posix_time::millisec(250)),
	m_hapticsPull(_io->GetIOService(), boost::posix_time::millisec(5)),
	m_commandPull(_io->GetIOService(), boost::posix_time::millisec(50)),
	m_trackingPush(_io->GetIOService(), boost::posix_time::millisec(10)),
	m_imus(),
	m_cachedTracking({})

{
	using namespace boost::log;

	typedef sinks::synchronous_sink<MyTestLog> sink_t;

	boost::shared_ptr<sink_t> sink(new sink_t());
	sink->locked_backend()->ProvideMessenger(m_messenger);

	core::get()->add_sink(sink);
	
	m_hardware.RegisterPacketCallback(SuitPacket::PacketType::ImuData, [this](auto packet) {
		m_imus.ConsumePacket(packet); 
	});

	BOOST_LOG_TRIVIAL(info) << "[DriverMain] Booting";

	m_hardware.RegisterPacketCallback(SuitPacket::PacketType::SuitVersion, [this](auto packet) {
		SuitVersionInfo version(packet);
		BOOST_LOG_TRIVIAL(info) << "[DriverMain] Suit diagnostics: Running firmware v" << version.Major << "." << version.Minor;

		if (version.Major == 2 && version.Minor == 5) {

			m_imus.AssignMapping(1, Imu::Left_Upper_Arm);
			m_imus.AssignMapping(3, Imu::Right_Upper_Arm);
			m_imus.AssignMapping(0, Imu::Chest);
		}
		else if (version.Major == 2 && version.Minor == 3) {
			m_imus.AssignMapping(0, Imu::Chest);
		}
		else if (version.Major == 2 && version.Minor == 4) {
			m_imus.AssignMapping(3, Imu::Chest);
		}

		m_hardware.ReadDriverData(Location::Chest_Left);
	});

	m_hardware.RegisterPacketCallback(SuitPacket::PacketType::DrvStatus, [&](auto packet) {
		extractDrvData(packet);

	});

}

Driver::~Driver()
{
}

bool Driver::StartThread()
{

	m_hapticsPull.SetEvent([this]() { handleHaptics(); });
	m_hapticsPull.Start();

	m_statusPush.SetEvent([this]() { handleStatus(); });
	m_statusPush.Start();

	m_commandPull.SetEvent([this]() {handleCommands(); });
	m_commandPull.Start();

	m_trackingPush.SetEvent([this]() {handleTracking(); });
	m_trackingPush.Start();
	return true;
}

bool Driver::Shutdown()
{
	BOOST_LOG_TRIVIAL(info) << "[DriverMain] Shutting down";

	m_statusPush.Stop();
	m_hapticsPull.Stop();
	m_commandPull.Stop();
	m_trackingPush.Stop();
	m_messenger.Disconnect();
	
	_io->Shutdown();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	return true;
}

void Driver::handleHaptics()
{
	if (auto commands = m_messenger.ReadHaptics()) {
		for (const auto& command : *commands) {
			m_hardware.ReceiveExecutionCommand(command);
		}
	}
}

void Driver::handleStatus()
{
	m_messenger.WriteSuits(m_hardware.PollDevice());
}

void DoForEachBit(std::function<void(Location l)> fn, uint32_t bits) {
	for (uint32_t bit = 1; bits >= bit; bit *= 2) if (bits & bit) fn(Location(bit));

}
void Driver::handleCommands()
{
	if (auto commands = m_messenger.ReadCommands()) {
		for (const auto& command : *commands) {
			switch (command.command()) {
			case NullSpaceIPC::DriverCommand_Command_DISABLE_TRACKING:
				BOOST_LOG_TRIVIAL(info) << "[DriverMain] Disabling tracking";

				m_hardware.DisableTracking();
				break;
			case NullSpaceIPC::DriverCommand_Command_ENABLE_TRACKING:
				BOOST_LOG_TRIVIAL(info) << "[DriverMain] Enabling tracking";
				m_hardware.EnableTracking();
				break;
			case NullSpaceIPC::DriverCommand_Command_ENABLE_AUDIO:
				BOOST_LOG_TRIVIAL(info) << "[DriverMain] Enabling audio mode for all pads";
				FirmwareInterface::AudioOptions options;
				options.AudioMin =  command.params().at("audio_min");
				options.AudioMax = command.params().at("audio_max");
				options.PeakTime = command.params().at("peak_time");
				options.Filter = command.params().at("filter");
				for (int loc = (int)Location::Lower_Ab_Right; loc != (int)Location::Error; loc++) {
					m_hardware.EnableAudioMode(static_cast<Location>(loc), options);
				}
				break;
			case NullSpaceIPC::DriverCommand_Command_DISABLE_AUDIO:
				BOOST_LOG_TRIVIAL(info) << "[DriverMain] Disabling audio mode for all pads";
			
				for (int loc = (int)Location::Lower_Ab_Right; loc != (int)Location::Error; loc++) {
					m_hardware.EnableIntrigMode(static_cast<Location>(loc));
				}
				break;
			case NullSpaceIPC::DriverCommand_Command_RAW_COMMAND:
				BOOST_LOG_TRIVIAL(info) << "[DriverMain] Submitting raw command to the suit";
				m_hardware.RawCommand(command.raw_command());
				break;
			case NullSpaceIPC::DriverCommand_Command_DUMP_DEVICE_DIAGNOSTICS:
				BOOST_LOG_TRIVIAL(info) << "[DriverMain] Requesting device diagnostics..";
				for (int loc = (int)Location::Lower_Ab_Right; loc != (int) Location::Error; loc++) {
					m_hardware.ReadDriverData(static_cast<Location>(loc));
				}
				break;
			default: 
				break;
			}
		}
	}
}

void Driver::handleTracking()
{
	NullSpace::SharedMemory::TrackingUpdate update = m_cachedTracking;

	if (auto quat = m_imus.GetOrientation(Imu::Chest)) {
		update.chest = *quat;
	}

	if (auto quat = m_imus.GetOrientation(Imu::Left_Upper_Arm)) {
		update.left_upper_arm = *quat;
	}

	if (auto quat = m_imus.GetOrientation(Imu::Right_Upper_Arm)) {
		update.right_upper_arm = *quat;
	}
	
	m_cachedTracking = update;

	m_messenger.WriteTracking(update);
}

