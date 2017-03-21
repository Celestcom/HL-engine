#include "StdAfx.h"
#include "BoostSerialAdapter.h"
#include <iostream>
#include <future>
#include <boost/asio/use_future.hpp>
#include "enumser.h"
#include "Locator.h"
#include "AsyncTimeout.h"


uint8_t BoostSerialAdapter::m_pingData[7] = { 0x24, 0x02, 0x02, 0x07, 0xFF, 0xFF, 0x0A };

BoostSerialAdapter::BoostSerialAdapter(std::shared_ptr<IoService> ioService) :
	suitDataStream(std::make_shared<Buffer>(4096)), 
	port(nullptr),
	_io(ioService->GetIOService()),
	_suitReconnectionTimeout(boost::posix_time::milliseconds(50)),
	_initialConnectTimeout(boost::posix_time::milliseconds(300)),
	_suitReconnectionTimer(_io),
	_monitor(ioService->GetIOService(), port)
{
	std::fill(m_data, m_data + INCOMING_DATA_BUFFER_SIZE, 0);
	_monitor.SetDisconnectHandler([this]() { beginReconnectionProcess(); });
}

void BoostSerialAdapter::Connect()
{
	//When we first connect, we don't want any delays
	scheduleImmediateSuitReconnect();
}

void BoostSerialAdapter::Disconnect()
{
	try {
		if (port->is_open()) {
			port->close();
		}
	}
	catch (const boost::system::error_code& ec) {
		//log this
	}
}

void BoostSerialAdapter::Write(std::shared_ptr<uint8_t*> bytes, std::size_t length, std::function<void(const boost::system::error_code&, std::size_t)> cb)
{
	if (this->port && this->port->is_open()) {
		this->port->async_write_some(boost::asio::buffer(*bytes, length), cb);
	}
}
void BoostSerialAdapter::Write(std::shared_ptr<uint8_t*> bytes, std::size_t length)
{
	if (this->port && this->port->is_open()) {
		this->port->async_write_some(boost::asio::buffer(*bytes, length),
			[bytes](const boost::system::error_code& error, std::size_t bytes_transferred) {
			if (error) {
				Locator::Logger().Log("Adapter", "Failed to write to suit!", LogLevel::Error);
			}
		});
	}
}


void BoostSerialAdapter::kickoffSuitReading()
{
	if (!this->port || !this->port->is_open()) {
		return;
	}

	this->port->async_read_some(boost::asio::buffer(m_data, INCOMING_DATA_BUFFER_SIZE), [this]
		(auto error, auto bytes_transferred) {
		
			if (!error) {
				
				if (!isPingPacket(m_data, bytes_transferred)) {
					//if it's not a ping packet, put it into the data stream. Don't want pings cluttering stuff up.
					suitDataStream->push(m_data, bytes_transferred);
					std::fill(m_data, m_data + INCOMING_DATA_BUFFER_SIZE, 0);
				}
				else {
					_monitor.ReceivePing();
				}

				kickoffSuitReading();
			}	
		}
	);
	
}







void BoostSerialAdapter::beginReconnectionProcess() {
	std::cout << "Disconnected..\n";
	Locator::Logger().Log("Adapter", "Reconnecting..");
	_isResetting = true;
	scheduleImmediateSuitReconnect();
}
void BoostSerialAdapter::endReconnectionProcess()
{
	_isResetting = false;
	_monitor.BeginMonitoring();
	kickoffSuitReading();
}



std::shared_ptr<Buffer> BoostSerialAdapter::GetDataStream()
{
	return this->suitDataStream;
}

bool BoostSerialAdapter::IsConnected() const
{
	return  !_isResetting && this->port && this->port->is_open();
}




BoostSerialAdapter::~BoostSerialAdapter()
{
	Disconnect();
}

/*
This method kicks off the connection to the serial ports.

It attempts to check all available ports, sending a ping to each. If it receives the magic response,
it will use that port. 
*/

void BoostSerialAdapter::testAllAsync(const boost::system::error_code& ec) {

	if (ec) {
		return;
	}
	//First, we retrieve all the available ports. This is windows-only. 
	CEnumerateSerial::CPortsArray ports;
	CEnumerateSerial::CNamesArray names;
	if (!CEnumerateSerial::UsingQueryDosDevice(ports)) {
		Locator::Logger().Log("Adapter", "No ports available on system. Check Device Manager for available devices.", LogLevel::Warning);
	}

	//Then, we append "COM" to each of them.
	std::vector<std::string> portNames;
	for (std::size_t i = 0; i < ports.size(); ++i) {
		portNames.push_back("COM" + std::to_string(ports[i]));
		Locator::Logger().Log("Adapter", portNames.back());
	}
	
	//Finally, we post the testOnePort method to the IO service. This method will test each port
	//sequentially until it finds the correct port or runs out of ports, in which case it will
	//start over again after a short delay.
	_io.post([this, portNames]() { testOnePort(portNames); });

}


//This is quite a large method. Not sure if splitting into two would 
//help with understanding it, but if so then do it!
//This method tries to connect to each port in the portNames list one after the other
void BoostSerialAdapter::testOnePort(std::vector<std::string> portNames) {

	//If the list of ports is empty, we've exhausted all our options.
	//Therefore, we must wait a bit and then try everything again
	if (portNames.empty()) {
		scheduleDelayedSuitReconnect();
		return;
	}

	//Call the port's destructor and reinstantiate it
	port = std::make_unique<boost::asio::serial_port>(_io);

	//Grab the next portname and remove it from the vector
	std::string portName = portNames.back();
	portNames.pop_back();

	//Attempt to simply open up the serial port. If that doesn't work, try the next..
	if (!tryOpenPort(*port, portName)) {
		_io.post([portNames, this] {testOnePort(portNames); }); 
		return;
	}

	//Okay, the port is open. Now send a ping to it.
	port->async_write_some(boost::asio::buffer(m_pingData, 7), [](auto ec, auto bytes_transferred) {});
	
	//Read from the port with a specified timeout (if the timeout is reached, we move on to the next) 
	auto timedReader = std::make_shared<AsyncTimeout>(_io, boost::posix_time::millisec(_initialConnectTimeout));
	
	timedReader->OnTimeout([&](){
		//if we end up timing out, we should close the port if it was opened
		if (port && port->is_open()) {
			try {
				port->close();
			}
			catch (const boost::system::system_error& ec) {
				//intentionally left blank
			}
		}
	});

	//Begin the timeout countdown
	timedReader->Go();
	

	port->async_read_some(boost::asio::buffer(m_data, INCOMING_DATA_BUFFER_SIZE), [this, portNames, timedReader] 
		(auto ec, auto bytes_transferred) {
		
		//Stop the timed read from potentially expiring
		timedReader->Cancel();

		//Success case
		if (!ec) {
			assert(port && port->is_open());
			if (isPingPacket(m_data, bytes_transferred)) {
				std::cout << "Connected.\n";
				_io.post([this] { endReconnectionProcess(); });
				return;
			}
		}

		//got some kind of error reading, or it wasn't a ping packet, so try the next
		_io.post([portNames, this]() { testOnePort(portNames);});
	});
}

bool BoostSerialAdapter::tryOpenPort(boost::asio::serial_port& port, std::string portName)
{
	Locator::Logger().Log("Adapter", "About to try and open port, let's see if it throws..");

	try {
		port.open(portName);
		if (!port.is_open()) {
			Locator::Logger().Log("Adapter", "It didn't throw but it's not open. Calling testOnePort again");

			return false;
		}
	}
	catch (boost::system::system_error& ec) {
		Locator::Logger().Log("Adapter", std::string("Yup it threw this error: ") + ec.what());

		return false;
	}

	Locator::Logger().Log("Adapter", "It opened successfully");

	return true;
}

void BoostSerialAdapter::scheduleImmediateSuitReconnect()
{
	_suitReconnectionTimer.expires_from_now(boost::posix_time::millisec(0));
	_suitReconnectionTimer.async_wait([this](auto err) {testAllAsync(err); });
}

bool BoostSerialAdapter::isPingPacket(uint8_t* data, std::size_t length)
{
	if (length < 3) {
		return false;
	}

	return (
			data[0] == '$'
		 && data[1] == 0x02
		 && data[2] == 0x02
	);
		
}

void BoostSerialAdapter::scheduleDelayedSuitReconnect()
{

	_suitReconnectionTimer.expires_from_now(_suitReconnectionTimeout);
	_suitReconnectionTimer.async_wait([this](auto err) {testAllAsync(err); });

}



