#pragma once
#include "Enums.h"

class HapticEvent
{
public:
	HapticEvent(Effect effect, float duration);
	~HapticEvent();
	bool Dirty;
	Effect Effect;
	float Duration;
	float TimeElapsed;
	::Duration DurationType() const;
};

