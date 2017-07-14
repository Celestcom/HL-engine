#pragma once

#include "Enums.h"

#include "ZoneModel.h"
#include "RtpModel.h"
#include <boost/uuid/random_generator.hpp>
#include "HardwareCommands.h"
#include "../include/events/BriefTaxel.h"
#include "../include/events/LastingTaxel.h"
#include "../include/events/PlaybackEvent.h"
#include "../include/events/RealtimeEvent.h"

#include "PluginAPI.h"

class Hardlight_Mk3_ZoneDriver {
public:
	CommandBuffer update(float dt);

	Hardlight_Mk3_ZoneDriver(Location area);

	Location GetLocation();

	//boost::optional<HapticDisplayInfo> QueryCurrentlyPlaying();
	void consume(nsvr_cevent_brief_haptic* event);
	void consume(nsvr_cevent_lasting_haptic* event);
	void consume(nsvr_cevent_playback_statechange* event);
	void consume(nsvr_cevent_realtime_request* event);

	void consume(const NSVR_BriefTaxel* taxel);
	void consume(const NSVR_LastingTaxel * haptic);
	void consume(const NSVR_PlaybackEvent* event);
	void consume(const NSVR_RealtimeEvent* realtime);
	//void realtime(uint16_t volume);
private:
	::Location m_area;

	ZoneModel m_retainedModel;
	RtpModel m_rtpModel;
	
	boost::uuids::random_generator m_gen;
	enum class Mode {Retained, Realtime};
	void transitionInto(Mode mode);
	Mode m_currentMode;
	ParentId m_parentId;
	std::mutex m_mutex;
	CommandBuffer m_commands;
};