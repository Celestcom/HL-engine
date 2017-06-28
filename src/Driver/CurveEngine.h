#pragma once
#include "protobuff_defs/HighLevelEvent.pb.h"
#include <stdint.h>
#include <unordered_map>
class PluginManager;
class HardwareInterface;


class PluginDispatcher;
class CurveEngine {
public:
	void GenerateCurve(uint64_t id, const NullSpaceIPC::CurveHaptic& haptic);
	void Update(float dt);
	CurveEngine(PluginManager& manager, HardwareInterface& hardware);
private:
	PluginManager& m_dispatcher;
	void changePlaybackState(uint64_t id, NullSpaceIPC::PlaybackEvent_Command);

	

};