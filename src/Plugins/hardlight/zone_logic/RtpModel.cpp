#include "stdafx.h"
#include "RtpModel.h"
#include <boost/log/trivial.hpp>


RtpModel::RtpModel(Location area) : 
	location(area), 
	volume(0), 
	volumeValueProtector(),
	m_samples(2048),
	volumeCommand(boost::none)
{
}

void RtpModel::Put(BufferedEvent event)
{
	//naive at first. Just putting all the samples in the queue. No layering, etc.
	auto data = event.Data();
	
	for (double d : data) {
		m_samples.push(d);
	}
}

void RtpModel::ChangeVolume(int newVolume)
{
	

	std::lock_guard<std::mutex> guard(volumeValueProtector);

	if (newVolume != volume) {
		volume = newVolume;
	
		volumeCommand = PlayVol(location, volume / 2);
	}
	else {
		volumeCommand = boost::none;
	}

}

CommandBuffer RtpModel::Update(float dt)
{
	CommandBuffer commands;

	std::lock_guard<std::mutex> guard(volumeValueProtector);

	//BOOST_LOG_TRIVIAL(info) << "[RtpModel] Size of the queue: " << m_samples.read_available();
	if (!m_samples.empty()) {
		double vol = m_samples.front();
		m_samples.pop();
		commands.push_back(PlayVol(location, vol * 255));
	}

/*
	if (volumeCommand) {
		commands.push_back(std::move(*volumeCommand));
		volumeCommand = boost::none;
	}
*/
	return commands;
}

int RtpModel::GetVolume()
{
	return volume;
}