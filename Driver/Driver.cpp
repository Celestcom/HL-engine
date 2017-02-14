#include "stdafx.h"
#include "Driver.h"
#include "IoService.h"
#include "ClientMessenger.h" 
#include "IntermediateHapticFormats.h"
#include "Encoder.h"
Driver::Driver() :
	_io(new IoService()),
	_running(false),
	_pollTimer(_io->GetIOService()),
	_pollInterval(50),
	_hardware(_io),
	_messenger(_io->GetIOService()),
	_cMessenger(_io->GetIOService())

{
/*	_messenger.Receive([this](void const* data, std::size_t size) { 
		flatbuffers::Verifier verifier(reinterpret_cast<uint8_t const*>(data), size);
		if (NullSpace::HapticFiles::VerifyExecutionCommandBuffer(verifier)) {
			auto packet = NullSpace::HapticFiles::GetExecutionCommand(data);
			_hardware.ReceiveExecutionCommand(_encoder.Decode(packet));
		}
	
	
	
	});
	*/
	_pollTimer.expires_from_now(_pollInterval);
	_pollTimer.async_wait(boost::bind(&Driver::_UpdateLoop, this));

}

Driver::~Driver()
{
	if (_workThread.joinable()) {
		_workThread.join();
	}
}

bool Driver::StartThread()
{
	
	_running = true;
	//_workThread = std::thread(boost::bind(&Driver::_UpdateLoop, this));

//	_pollTimer.expires_from_now(_pollInterval);
	//_pollTimer.async_wait(boost::bind(&Driver::_PollHandler, this, boost::asio::placeholders::error));

	
	return true;
}

bool Driver::Shutdown()
{
	_running.store(false);
//	_messenger.Disconnect();
	
	if (_messengerThread.joinable()) {
		_messengerThread.join();
	}
	//_pollTimer.cancel();
	
	_io->Shutdown();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	return true;
}

void Driver::_UpdateLoop()
{
	ExecutionCommand c;
	c.Command = NullSpace::HapticFiles::PlayCommand_PLAY;
	c.Effect = 0;
	
	c.Location = counter;
	counter++;
	if (counter > 15) {
		counter = 1;
	}
	//Encoder encoder;

	//encoder.AquireEncodingLock();
//	auto offset = encoder.Encode(c);

	//encoder._finalize(offset, [&messenger, &encoder, this](uint8_t* data, int size) {
	//	messenger.Send(data, size);
		//encoder.ReleaseEncodingLock();

	//	_pollTimer.expires_from_now(_pollInterval);
	//	_pollTimer.async_wait(boost::bind(&Driver::_UpdateLoop, this));


	//});

}

