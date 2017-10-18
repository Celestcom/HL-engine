#pragma once
#include "SuitPacket.h"
#include <unordered_map>
#include <functional>

using namespace std;

class PacketDispatcher
{
public:
	using OnReceivePacketFunc = std::function<void(packet)>;
	PacketDispatcher();
	void Dispatch(packet packet);
	void AddConsumer(PacketType ptype, OnReceivePacketFunc packetFunc);
private:
	unsigned int _dispatchLimit;
	std::unordered_map<PacketType, std::vector<OnReceivePacketFunc>> _consumers;
};

