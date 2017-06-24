#include "stdafx.h"
#include "HardwareInterface.h"
#include "BoostSerialAdapter.h"
#include "IoService.h"
#include "Locator.h"

HardwareInterface::HardwareInterface(std::shared_ptr<IoService> ioService, RegionRegistry& registry) :
	m_adapter(std::make_unique<BoostSerialAdapter>(ioService->GetIOService())),

	m_firmware(m_adapter, ioService->GetIOService()),
	m_monitor(std::make_shared<KeepaliveMonitor>(ioService->GetIOService(), m_firmware)),

	m_synchronizer(std::make_unique<Synchronizer>(m_adapter->GetDataStream(), m_dispatcher, ioService->GetIOService())),
	
	m_running(true),
	m_registry(registry)
	

{
	//m_adapter->SetMonitor(m_monitor);
	//m_adapter->Connect();
	//m_synchronizer->BeginSync();
	

}


HardwareInterface::~HardwareInterface()
{
	m_running = false;
	
}

SuitsConnectionInfo HardwareInterface::PollDevice()
{
	SuitsConnectionInfo info = {};
	info.timestamp = std::time(nullptr);
	info.SuitsFound[0] = true;
	info.Suits[0].Id = 1;
	info.Suits[0].Status = m_adapter->IsConnected() ? 
		NullSpace::SharedMemory::Connected : 
		NullSpace::SharedMemory::Disconnected;

	return info;
}

void HardwareInterface::ResetDrivers()
{
	m_firmware.ResetDrivers();
}

void HardwareInterface::ReadDriverData(Location loc) {
	//m_firmware.ReadDriverData(loc, Register::Control);
	//m_firmware.ReadDriverData(loc, Register::RtpVol);
	m_firmware.ReadDriverData(loc, Register::Status);
	//m_firmware.ReadDriverData(loc, Register::WaveForm);


}

void HardwareInterface::EnableTracking()
{

	m_firmware.RequestSuitVersion();
	m_firmware.EnableTracking();
	
}

void HardwareInterface::DisableTracking()
{
	m_firmware.DisableTracking();
}

void HardwareInterface::EnableAudioMode(Location pad, const FirmwareInterface::AudioOptions& options)
{
	m_firmware.EnableAudioMode(pad, options);
}

void HardwareInterface::EnableIntrigMode(Location pad)
{
	m_firmware.EnableIntrigMode(pad);
}

void HardwareInterface::EnableRtpMode(Location pad)
{
	m_firmware.EnableRtpMode(pad);
}

void HardwareInterface::RequestSuitVersion()
{
	m_firmware.RequestSuitVersion();
}

void HardwareInterface::RegisterPacketCallback(SuitPacket::PacketType p, std::function<void(packet p)> func)
{
	m_dispatcher.AddConsumer(p, func);
}

void HardwareInterface::RawCommand(const std::string & command)
{
	auto data = reinterpret_cast<const uint8_t*>(command.data());
	m_firmware.RawCommand(data, command.size());
}

void HardwareInterface::generateLowLevelSimpleHapticEvents(const NullSpaceIPC::HighLevelEvent& event)
{
	const auto& simple_event = event.simple_haptic();
	const std::string& region = event.region();

	if (simple_event.duration() == 0.0f) {
		nsvr::events::BriefTaxel taxel = { 0 };
		taxel.Effect = simple_event.effect();
		taxel.Strength = simple_event.strength();
		m_registry.Activate(region, "brief-taxel", AS_TYPE(NSVR_BriefTaxel, &taxel));
	}
	else {
		nsvr::events::LastingTaxel taxel = { 0 };
		taxel.Effect = simple_event.effect();
		taxel.Strength = simple_event.strength();
		taxel.Duration = simple_event.duration();
		m_registry.Activate(region, "lasting-taxel", AS_TYPE(NSVR_LastingTaxel, &taxel));
	}
}

