#include "BoostSerialAdapter.h"
#include "enumser.h"
#include <iostream>
#include "BlockingReader.h"
#include <future>


bool BoostSerialAdapter::Connect()
{
	return this->autoConnectPort();
}

void BoostSerialAdapter::Disconnect()
{
	if (port->is_open()) {
		port->close();
	}
}

void BoostSerialAdapter::Write(uint8_t bytes[], std::size_t length)
{
	if (this->port && this->port->is_open()) {
		char *chars = reinterpret_cast<char*>(bytes);
		this->port->async_write_some(boost::asio::buffer(bytes, length),
			[&](const boost::system::error_code& error, std::size_t bytes_transferred) {
			if (error) {
				std::cout << "Error writing bytes! " << error.message() << '\n';
			}
		});
	}
}
void BoostSerialAdapter::read_handler(boost::system::error_code ec, std::size_t length) {
	if (!ec && length > 0) {
		_keepaliveTimer.cancel();
		this->copy_data_to_circularbuff(length);
		doSuitRead();
	}
	
}

void BoostSerialAdapter::doSuitRead()
{
	if (this->port->is_open()) {
		this->port->async_read_some(boost::asio::buffer(_data, INCOMING_DATA_BUFFER_SIZE),
			boost::bind(&BoostSerialAdapter::read_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}	
}
void BoostSerialAdapter::startPingTimer() {
		_keepaliveTimer.expires_from_now(_keepaliveInterval);

	_keepaliveTimer.async_wait(boost::bind(&BoostSerialAdapter::doKeepAlivePing, this));
}
void BoostSerialAdapter::suitReadCancel(boost::system::error_code ec)
{
	
	if (ec) {
		_pingTime = _keepaliveInterval.total_milliseconds() - _keepaliveTimer.expires_from_now().total_milliseconds();
	
		_sendPingTimer.expires_from_now(_pingTimeout);
		_sendPingTimer.async_wait(boost::bind(&BoostSerialAdapter::startPingTimer, this));
		return;
	}
	std::cout << "Timed out!" << '\n';

	//We have to do a silly reset dance with the main thread because I cannot reset the io service from a handler,
	//as far as I can tell. If this _can_ be done, please replace!
	//Relevant items: _resetMutex, _needsReset, and the logic in the Engine update loop which tests if the adapter needs
	//to be reset.

	std::lock_guard<std::mutex> lock(_resetMutex);
	if (!_needsReset) {
		_needsReset = true;
	}
	
}

void BoostSerialAdapter::DoReset() {
	//Part of the reset dance is locking the flag to reset, because I am not confident enough in the design 
	//to guarantee that the suitReadCancel may not be called from another thread in the future.
	//Currently it should not need the lock because we have only one IO thread, and all handlers are guarantee to run non concurrently.
	_resetMutex.lock();
	_needsReset = false;
	_resetMutex.unlock();
	this->reconnectSuit();
	
}
void BoostSerialAdapter::reconnectSuit() {
	std::cout << "Attempting to auto reconnect.." << '\n';
	_isResetting = true;
	if (this->autoConnectPort()) {
		this->BeginRead();
		std::cout << "... auto reconnected!" << '\n';

		_isResetting = false;
	}
	else {
		std::lock_guard<std::mutex> lock(_resetMutex);
		_needsReset = true;
	}
}
void BoostSerialAdapter::BeginRead()
{
	this->doKeepAlivePing();
	doSuitRead();

}
void BoostSerialAdapter::copy_data_to_circularbuff(std::size_t length) {
	//for (std::size_t i = 0; i < length; ++i) {
		//suitDataStream->push_front(_data[i]);
		//suitDataStream->push(_data[i]);
	//}
	suitDataStream->push(_data, length);

	
	std::fill(_data,_data+INCOMING_DATA_BUFFER_SIZE, 0);
}
void BoostSerialAdapter::write_handler(boost::system::error_code ec, std::size_t length) {

}

void BoostSerialAdapter::doKeepAlivePing()
{
	char ping[] = { 0x24, 0x02, 0x02, 0x07, 0xFF, 0xFF, 0x0A };
	this->port->async_write_some(boost::asio::buffer(&ping, 7), boost::bind(&BoostSerialAdapter::write_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	_keepaliveTimer.expires_from_now(_keepaliveInterval);
	_keepaliveTimer.async_wait(boost::bind(&BoostSerialAdapter::suitReadCancel, this, boost::asio::placeholders::error));
	
}

bool BoostSerialAdapter::Connect(std::string name)
{
	return false;
}

std::shared_ptr<Buffer> BoostSerialAdapter::GetDataStream()
{
	return this->suitDataStream;
}

bool BoostSerialAdapter::IsConnected() const
{
	return  !_isResetting && this->port && this->port->is_open();
}



BoostSerialAdapter::BoostSerialAdapter(std::shared_ptr<IoService> ioService) :
	suitDataStream(std::make_shared<Buffer>(4096)), port(nullptr), _io(ioService->GetIOService()),
	_keepaliveTimer(*_io, _keepaliveInterval),
	_ioService(ioService),
	_sendPingTimer(*_io, _pingTimeout)

{
	std::fill(_data, _data + INCOMING_DATA_BUFFER_SIZE, 0);

}


BoostSerialAdapter::~BoostSerialAdapter()
{
	if (port && port->is_open()) {
		port->close();
	}
}


bool BoostSerialAdapter::doHandshake( std::string portName) {

	//First, we attempt to create and connect to the port. 
	//Then, we send a short ping and see if we receive a response.

	if (this->createPort(portName)) {
		char ping[] = { 0x24, 0x02, 0x02, 0x07, 0xFF, 0xFF, 0x0A };
		this->port->async_write_some(boost::asio::buffer(&ping, 7), boost::bind(&BoostSerialAdapter::write_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		//Don't want to deal with more async handlers here, so use a std::future to wait for a couple hundred millis
		//(suit takes about 30ms first ping)
		std::future<std::size_t> length = port->async_read_some(boost::asio::buffer(_data, INCOMING_DATA_BUFFER_SIZE), boost::asio::use_future);
		auto status = length.wait_for(_initialConnectTimeout);
		switch (status) {
		case std::future_status::ready:
			return true;
		case std::future_status::timeout:
			return false;
		case std::future_status::deferred:
			return false;
		default:
			return false;
		}
	}
	return false;
}
bool BoostSerialAdapter::autoConnectPort()
{

	CEnumerateSerial::CPortsArray ports;
	CEnumerateSerial::CNamesArray names;
	if (!CEnumerateSerial::UsingQueryDosDevice(ports)) {
		std::cout << "No ports available on system." << "\n";
		return false;
	}
	for (std::size_t i = 0; i < ports.size(); ++i) {
		std::string strname = "COM" + std::to_string(ports[i]);
		if (this->doHandshake(strname)) {
			return true;
		}
	}
	return false;

}

bool BoostSerialAdapter::createPort(std::string name)
{

	//Need to close old port if it is open
	if (this->port && this->port->is_open()) {
		this->port->close();
		//Because weird serial port quirks, we need to reset the entire IO service. 
		//I would like to reset this from another thread ideally, because this could cause the Main thread to slow down
		_ioService->RestartIOService();
		if (this->port->is_open()) {
			std::cout << "Major error: After resetting IO service, the port was still open. Talk to casey@nullspacevr.com\n";
		}
	}
	this->port = std::make_unique<boost::asio::serial_port>(*_io);

	try {
		this->port->open(name);
		return this->port->is_open();
	}
	catch (boost::system::system_error& e) {
		//This is the most likely case for failing when opening a port. If there is no device connected, we hit
		//this case.
		return false;
	}

}

