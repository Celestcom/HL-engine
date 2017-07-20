#include "stdafx.h"
#include "LiveBasicHapticEvent.h"
#include <cmath>


LiveBasicHapticEvent::LiveBasicHapticEvent() :
	parentId(),
	uniqueId(),
	currentTime(0),
	isPlaying(false),
	eventData()
{
	if (std::abs(eventData.duration - 0.25f) <= 0.001f) {
		eventData.duration = 0;
	}
}

LiveBasicHapticEvent::LiveBasicHapticEvent(ParentId  parentId, boost::uuids::uuid uniqueId, BasicHapticEventData data) :
	parentId(parentId),
	uniqueId(uniqueId),
	currentTime(0),
	isPlaying(true),
	eventData(std::move(data))

{
	if (std::abs(eventData.duration - 0.25f) <= 0.001f) {
		eventData.duration = 0;
	}
}

const BasicHapticEventData & LiveBasicHapticEvent::Data() const
{
	return eventData;
}

bool LiveBasicHapticEvent::operator==(const LiveBasicHapticEvent & other) const
{
	return uniqueId == other.uniqueId;
}

void LiveBasicHapticEvent::update(float dt)
{
	if (isPlaying) {
		currentTime += dt;
	}
}

bool LiveBasicHapticEvent::isFinished() const
{
	return currentTime >= eventData.duration;
}

bool LiveBasicHapticEvent::isContinuous() const
{
	return !isOneshot();
}

bool LiveBasicHapticEvent::isOneshot() const
{
	return eventData.duration == 0;
}

bool LiveBasicHapticEvent::isChildOf(ParentId parentId) const
{
	return nsvr_playback_handle_equal(this->parentId, parentId);
	//return this->parentId == parentId;
}

