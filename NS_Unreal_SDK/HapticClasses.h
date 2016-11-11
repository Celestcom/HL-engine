#pragma once
#include "Enums.h"
#include <vector>
#include "ITimeOffset.h"
#include <iostream>
#include "IntermediateHapticFormats.h"

class HapticEffect :public ITimeOffset {
public:
	HapticEffect(Effect effect, Location loc, float duration, float time, unsigned int priority);
	HapticEffect();
	~HapticEffect();
	Effect Effect;
	Location Location;
	float Duration;
	float Time;
	float GetTime() override;
	float OriginalTime;
	unsigned int Priority;
};

class HapticSequence
{
public:
	HapticSequence(std::vector<HapticEffect> effects);
	~HapticSequence();
	std::vector<HapticEffect> Effects;
};

class HapticFrame : public ITimeOffset
{
public:
	HapticFrame(float time, std::vector<JsonSequenceAtom> frame, AreaFlag area, unsigned int priority = 1);
	~HapticFrame();
	float Time;
	float GetTime() override;
	float OriginalTime;
	AreaFlag Area;
	unsigned int Priority;
	std::vector<JsonSequenceAtom> Frame;
	
};

class HapticSample : public ITimeOffset
{
public:
	unsigned int Priority;
	float Time;
	float OriginalTime;
	float GetTime() override;
	std::vector<HapticFrame> Frames;
	HapticSample(float time, std::vector<HapticFrame> frames, unsigned int priority);
	~HapticSample();
};

class Moment : public ITimeOffset
{
public:
	std::string Name;
	float Time;
	float GetTime() override;
	Side Side;
	Moment(std::string name, float t, ::Side side = Side::Inherit);
	Moment();
	~Moment();
};
