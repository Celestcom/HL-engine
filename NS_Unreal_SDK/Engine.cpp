#include "StdAfx.h"
#include "Engine.h"
#include "BoostSerialAdapter.h"

#include "TrackingUpdate_generated.h"
#include "Consumers\FifoConsumer.h"
#include "Consumers\SuitInfoConsumer.h"
#include "Consumers\SuitStatusConsumer.h"
#include "PlayableSequence.h"
#include "PlayablePattern.h"
#include "PlayableExperience.h"
#include "SuitDiagnostics.h"
Engine::Engine(std::shared_ptr<IoService> io, EncodingOperations& encoder, zmq::socket_t& socket) :
	//This contains all suit instructions, like GET_VERSION, PLAY_EFFECT, etc.
	_instructionSet(std::make_shared<InstructionSet>()),
	//This allows us to communicate with the suit. We are using Boost ASIO library right now, although
	//anything satisfying ICommunicationAdapter should work
	_adapter(std::shared_ptr<ICommunicationAdapter>(
		new BoostSerialAdapter(io)
	)),
	//The dispatcher takes packets and hands them out to components that subscribed to a certain packet type
	//For example, ImuConsumer is a subscriber to IMU Data packets
	_packetDispatcher(std::make_shared<PacketDispatcher>()),
	//The synchronizer reads from the raw suit stream and partitions it into packets. It can be upgraded to
	//do CRC, variable size packets, etc. 
	_streamSynchronizer(_adapter->GetDataStream(), _packetDispatcher),
	//The executor is responsible for executing haptic effects, and garbage collecting them when handles are
	//released.
	_executor(_instructionSet, std::make_unique<SuitHardwareInterface>(_adapter, _instructionSet, io->GetIOService())),
	//Since we want IMU data, we need an ImuConsumer to parse those packets into quaternions
	_imuConsumer(),
	//Since we only receive IMU data at a fixed rate, we should only dispatch it to the plugin at a fixed rate
	_trackingUpdateTimer(*io->GetIOService(), _trackingUpdateInterval),
	//Need an encoder to actually create the binary data that we send over the wire to the plugin
	_encoder(encoder),
	_socket(socket),
	//This is only present because we do not track separate state for each client. This wants to be its own
	//struct with all client related data. 
	_userRequestsTracking(false),
	_diagnostics()

{
	//Pulls all instructions, effects, etc. from disk
	if (!_instructionSet->LoadAll()) {
		std::cout << "Couldn't load configuration data, will exit after you hit [any key]" << '\n';
		std::cin.get();
		exit(0);
	}
	//bug: If you remove this initial connect, the port object could be empty. Should check for that.
	if (_adapter->Connect()) {
		std::cout << "> Connected to suit" << '\n';
	}
	else {
		std::cout << "> Unable to connect to suit." << "\n";
	}

	_diagnostics.OnReceiveVersion(boost::bind(&Engine::handleSuitVersionUpdate, this, _1));

	
	//Kickoff the communication adapter
	_adapter->BeginRead();
	_packetDispatcher->AddConsumer(SuitPacket::PacketType::ImuData, &_imuConsumer);
	_packetDispatcher->AddConsumer(SuitPacket::PacketType::FifoOverflow, _diagnostics.OverflowConsumer().get());
	_packetDispatcher->AddConsumer(SuitPacket::PacketType::SuitVersion, _diagnostics.InfoConsumer().get());
	_packetDispatcher->AddConsumer(SuitPacket::PacketType::SuitStatus, _diagnostics.StatusConsumer().get());

	_trackingUpdateTimer.async_wait(boost::bind(&Engine::sendTrackingUpdate, this));

}

void Engine::sendTrackingUpdate() {
	std::vector<std::tuple<Imu, Quaternion>> quats;
	std::vector<Imu> which = { Imu::Chest, Imu::Left_Upper_Arm, Imu::Right_Upper_Arm, Imu::Left_Forearm, Imu::Right_Forearm };
	for (const auto& imu : which) {
		if (boost::optional<Quaternion> q = _imuConsumer.GetOrientation(imu)) {
			quats.push_back(std::make_tuple(imu, q.get()));
		}
	}
	;	_encoder.AquireEncodingLock();
	_encoder.Finalize(_encoder.Encode(quats), [&](uint8_t* data, int size) {Wire::sendTo(_socket, data, size); });
	_encoder.ReleaseEncodingLock();
	
	_trackingUpdateTimer.expires_from_now(_trackingUpdateInterval);
	_trackingUpdateTimer.async_wait(boost::bind(&Engine::sendTrackingUpdate, this));
}


void Engine::handleSuitVersionUpdate(const SuitDiagnostics::VersionInfo & v)
{
	std::cout << "Major version: " << v.Major << "\nMinor version: " << v.Minor << "\n";
	if (v.Major == 2 && v.Minor == 4) {
		_imuConsumer.AssignMapping(3, Imu::Chest);
	}
	else if (v.Major == 2 && v.Minor == 3) {
		_imuConsumer.AssignMapping(0, Imu::Chest);
	}
	else if (v.Major == 2 && v.Minor == 5) {
		_imuConsumer.AssignMapping(1, Imu::Left_Upper_Arm);
		_imuConsumer.AssignMapping(3, Imu::Right_Upper_Arm);
		_imuConsumer.AssignMapping(0, Imu::Chest);

	}
}

void Engine::PlaySequence(const NullSpace::HapticFiles::HapticPacket& packet)
{
	auto name = packet.name()->str();
	auto rawSequenceData = static_cast<const NullSpace::HapticFiles::Node*>(packet.packet());
	auto location = AreaFlag(rawSequenceData->area());

	auto handle = packet.handle(); //converts from uint64 to uint32, maybe should check this

	auto decoded = EncodingOperations::DecodeSequence(rawSequenceData);
		
	//todo: figure out caching. If user changes a sequence's effects and the engine isn't
	//reloaded, then it is cached. Could do on create for very first time, cache
	                                                                        //default strength --V for now
	_executor.Create(handle, std::unique_ptr<IPlayable>(new PlayableSequence(decoded, location, 1.0)));
}

void Engine::PlayPattern(const NullSpace::HapticFiles::HapticPacket& packet)
{
	auto name = packet.name()->str();
	auto rawPatternData = static_cast<const NullSpace::HapticFiles::Node*>(packet.packet());
	auto handle = packet.handle();
	
	auto decoded = EncodingOperations::DecodePattern(rawPatternData);
	_executor.Create(handle, std::unique_ptr<IPlayable>(new PlayablePattern(decoded, _executor)));
}

void Engine::PlayExperience(const NullSpace::HapticFiles::HapticPacket& packet)
{
	auto name = packet.name()->str();
	auto rawExperienceData = static_cast<const NullSpace::HapticFiles::Node*>(packet.packet());
	auto handle = packet.handle();

	auto decoded = EncodingOperations::DecodeExperience(rawExperienceData);
	_executor.Create(handle, std::unique_ptr<IPlayable>(new PlayableExperience(decoded, _executor)));
}


void Engine::HandleCommand(const NullSpace::HapticFiles::HapticPacket & packet)
{
	auto decoded = EncodingOperations::Decode(static_cast<const NullSpace::HapticFiles::HandleCommand*>(packet.packet()));
	switch (decoded.Command) {
	case NullSpace::HapticFiles::Command_PAUSE:
		_executor.Pause(decoded.Handle);
		break;
	case NullSpace::HapticFiles::Command_PLAY:
		_executor.Play(decoded.Handle);
		break;
	case NullSpace::HapticFiles::Command_RESET:
		_executor.Reset(decoded.Handle);
		break;
	case NullSpace::HapticFiles::Command_RELEASE:
		_executor.Release(decoded.Handle);
	default:
		break;
	}
}

void Engine::EngineCommand(const NullSpace::HapticFiles::HapticPacket& packet) {
	auto decoded = EncodingOperations::Decode(static_cast<const NullSpace::HapticFiles::EngineCommandData*>(packet.packet()));
	switch (decoded.Command) {
	case NullSpace::HapticFiles::EngineCommand_ENABLE_TRACKING:
		_executor.Hardware()->EnableIMUs();
		_executor.Hardware()->RequestSuitVersion();
		break;
	case NullSpace::HapticFiles::EngineCommand_DISABLE_TRACKING:
		_executor.Hardware()->DisableIMUs();
		break;
	case NullSpace::HapticFiles::EngineCommand_CLEAR_ALL:
		_executor.ClearAll();
		break;
	case NullSpace::HapticFiles::EngineCommand_PAUSE_ALL:
		_executor.PauseAll();
		break;
	case NullSpace::HapticFiles::EngineCommand_PLAY_ALL:
		_executor.PlayAll();
		break;
	default:
		break;
	}
}
void Engine::Play(const NullSpace::HapticFiles::HapticPacket & packet)
{
	auto node = static_cast<const NullSpace::HapticFiles::Node*>(packet.packet());
	switch (node->type()) {
	case NullSpace::HapticFiles::NodeType_Sequence:
		PlaySequence(packet);
		break;
	case NullSpace::HapticFiles::NodeType_Pattern:
		PlayPattern(packet);
		break;
	case NullSpace::HapticFiles::NodeType_Experience:
		PlayExperience(packet);
		break;
	default:
		std::cout << "Unrecognized node type!" << node->type() << '\n';
		break;
	}
}
void Engine::EnableOrDisableTracking(const NullSpace::HapticFiles::HapticPacket & packet)
{
	_userRequestsTracking = EncodingOperations::Decode(static_cast<const NullSpace::HapticFiles::Tracking*>(packet.packet()));

	if (_userRequestsTracking) {
		_executor.Hardware()->EnableIMUs();
	}
	else {
		_executor.Hardware()->DisableIMUs();
	}
}

void Engine::Update(float dt)
{
	_executor.Update(dt);
	//todo: Raise event on disconnect and reconnect, so that engine can set the tracking to what the user requested
	if (_adapter->NeedsReset()) {
		_adapter->DoReset();
	}

	//read however many packets there are, with 500 as a hard maximum to keep the loop from
	//taking too much time
	int totalPackets = std::min(500, (int)_streamSynchronizer.PossiblePacketsAvailable());
	for (int i = 0; i < totalPackets; i++) {
		_streamSynchronizer.TryReadPacket();
	}


}

bool Engine::SuitConnected() const
{
	return _adapter->IsConnected();
}




Engine::~Engine()
{
}


