#pragma once
#include "Messenger.h"
#include "HardwareInterface.h"
#include "FirmwareInterface.h"
#include "Encoder.h"
class IoService;

class Driver {
public:
	Driver();
	~Driver();
	bool StartThread();
	bool Shutdown();
private:
	std::shared_ptr<IoService> _io;

	std::atomic<bool> _running;
	std::thread _workThread;
	std::thread _messengerThread;
	void _UpdateLoop();
	void _Update(void* data, std::size_t size);
	void _PollHandler(const boost::system::error_code& ec);
	boost::asio::deadline_timer _pollTimer;
	boost::posix_time::milliseconds _pollInterval;

	Messenger _messenger;
	HardwareInterface _hardware;
	Encoder _encoder;
};