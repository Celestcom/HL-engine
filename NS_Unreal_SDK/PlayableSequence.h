#pragma once
#include "IPlayable.h"
#include "TimeInstant.h"


class PlayableSequence : public IPlayable {
public:

	PlayableSequence(std::vector<JsonSequenceAtom>, AreaFlag loc, float strength);
	~PlayableSequence();
	void Play() override;
	void Reset(PriorityModel & model) override;
	void Pause(PriorityModel & model) override;
	void Update(float dt, PriorityModel & model, const std::unordered_map<std::string, Atom>&) override;
	uint32_t GetHandle() const override;
	float GetTotalPlayTime() const override;
	float CurrentTime() const override;
	bool IsPlaying() const override;
private:

	std::vector<Instant<JsonSequenceAtom>> _effects;
	std::vector<JsonSequenceAtom> _sourceOfTruth;
	std::vector<boost::uuids::uuid> _activeEffects;
	bool _paused;
	int _handle;
	float _strength;
	float _currentTime;
	AreaFlag _location;
};


