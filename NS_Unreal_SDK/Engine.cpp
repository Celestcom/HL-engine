#include "Engine.h"
#include "BoostSerialAdapter.h"
#include "Wire.h"
#include "EncodingOperations.h"
Engine::Engine(std::shared_ptr<boost::asio::io_service> io):
	_suitHardware(std::make_shared<SuitHardwareInterface>()),
	_adapter(std::shared_ptr<ICommunicationAdapter>(new BoostSerialAdapter(io, _suitHardware))),
	_packetDispatcher(_adapter->GetDataStream()),
	_streamSynchronizer(_adapter->GetDataStream(), std::shared_ptr<PacketDispatcher>(&_packetDispatcher)),
	_executor(_suitHardware)	

{

	if (!_adapter->Connect()) {
		std::cout << "Unable to connect to suit" << "\n";
	}
	else {
		std::cout << "Connected to suit" << "\n";
		_suitHardware->SetAdapter(_adapter);
		_adapter->Read();
	}
}


void Engine::PlaySequence(const NullSpace::HapticFiles::HapticPacket& packet)
{
	if (_hapticCache.ContainsSequence(packet.name()->str())) {
		_executor.Play(_hapticCache.GetSequence(packet.name()->str()));
	}
	else {
		auto decoded = EncodingOperations::Decode(static_cast<const NullSpace::HapticFiles::Sequence*>(packet.packet()));
		_hapticCache.AddSequence(packet.name()->str(), decoded);
		_executor.Play(decoded);
	}
}

void Engine::PlayPattern(const NullSpace::HapticFiles::HapticPacket& packet)
{
	if (_hapticCache.ContainsPattern(packet.name()->str())) {
		_executor.Play(_hapticCache.GetPattern(packet.name()->str()));
	}
	else {
		const NullSpace::HapticFiles::Pattern* packet_ptr = static_cast<const NullSpace::HapticFiles::Pattern*>(packet.packet());
		std::vector<HapticFrame> decoded = EncodingOperations::Decode(packet_ptr);
		_hapticCache.AddPattern(packet.name()->str(), decoded);
		_executor.Play(decoded);
	}
}

void Engine::PlayExperience(const NullSpace::HapticFiles::HapticPacket& packet)
{
	if (_hapticCache.ContainsExperience(packet.name()->str())) {
		_executor.Play(_hapticCache.GetExperience(packet.name()->str()));
	}
	else {
		auto decoded = EncodingOperations::Decode(static_cast<const NullSpace::HapticFiles::Experience*>(packet.packet()));
		_hapticCache.AddExperience(packet.name()->str(), decoded);
		_executor.Play(decoded);
	}
}

void Engine::PlayEffect(const NullSpace::HapticFiles::HapticPacket& packet)
{
	auto decoded = EncodingOperations::Decode(static_cast<const NullSpace::HapticFiles::HapticEffect*>(packet.packet()));
	_executor.Play(decoded);
}

void Engine::Update(float dt)
{
	_executor.Update(dt);
	//_adapter->Read();
	//_streamSynchronizer.TryReadPacket();

}

bool Engine::SuitConnected() const
{
	return _adapter->IsConnected();
}




Engine::~Engine()
{
}


