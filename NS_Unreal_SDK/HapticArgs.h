#pragma once
#include "Enums.h"
#include <string>

class HapticArgs
{
public:

	virtual ~HapticArgs()
	{
	}

	virtual int GetCombinedHash() const = 0;
};

class PatternArgs : public HapticArgs
{
public:
	PatternArgs(const std::string& name, Side side);
	~PatternArgs();
	Side Side;
	std::string Name;
	int GetCombinedHash() const override;
};

class SequenceArgs : public HapticArgs
{
public:
	SequenceArgs(const std::string& name, Location loc);
	~SequenceArgs();
	Location Location;
	std::string Name;
	int GetCombinedHash() const override;
};

class ExperienceArgs : public HapticArgs
{
public:
	ExperienceArgs(const std::string& name, Side side);
	~ExperienceArgs();
	Side Side;
	std::string Name;
	int GetCombinedHash() const override;
};