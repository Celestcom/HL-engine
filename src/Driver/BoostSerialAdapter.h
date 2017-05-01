#pragma once
#include "ICommunicationAdapter.h"
#include <boost\lockfree\spsc_queue.hpp>
#include "IoService.h"
#include "KeepaliveMonitor.h"


class BoostSerialAdapter : public std::enable_shared_from_this<BoostSerialAdapter>, public virtual ICommunicationAdapter
{
public:

	explicit BoostSerialAdapter(boost::asio::io_service& io);
	~BoostSerialAdapter();

	//Connect to the hardware through serial port
	void Connect() override;
	//Disconnect from hardware by closing the port
	void Disconnect() override;
	//Write bytes to the port, custom callback receives error codes and size written
	void Write(std::shared_ptr<uint8_t*> bytes, std::size_t length, std::function<void(const boost::system::error_code&, std::size_t)> cb) override;
	//Write bytes to the port, no callback
	void Write(std::shared_ptr<uint8_t*> bytes, std::size_t length) override;

	//Retrieve the base stream that the adapter is working on
	std::shared_ptr<Buffer> GetDataStream() override;

	//Check if the adapter is connected to the hardware
	bool IsConnected() const override;



private:

	//Note: I believe this class will need another refactoring when we support multiple suits

	/*General stuff*/
	

	//Our io_service, necessary for timers and ports
	boost::asio::io_service& m_io;

	//our serial port
	std::unique_ptr<boost::asio::serial_port> m_port;

	//Size of our incoming data buffer - should be tailored to the serial device
	const static unsigned int INCOMING_DATA_BUFFER_SIZE = 128;

	//our incoming data buffer 
	uint8_t m_data[INCOMING_DATA_BUFFER_SIZE];
	
	//our outgoing data buffer. Is lockfree but probably doesn't need to be.
	std::shared_ptr<Buffer> m_outputSuitData;

	//We flip this when resetting so that we can say the suit is "disconnected" in those periods
	bool _isResetting = false;

	//Handles pinging the suit 
	KeepaliveMonitor m_keepaliveMonitor;

	//Starts reading data from the suit
	void kickoffSuitReading();

	/* Testing port stuff */

	//Responsible for gathering the port names and calling testOnPort 
	void testAllPorts(const boost::system::error_code& ec);
	
	//Responsible for checking one port for the suit
	void testOnePort(std::vector<std::string> portNames);

	//Attempts to open a port with a given name
	bool tryOpenPort(boost::asio::serial_port& port, std::string portname);
	
	//Holds the common ping data
	static uint8_t m_pingData[7];

	//Checks if a given buffer is a ping packet
	bool isPingPacket(uint8_t* data, std::size_t length);

	//How long we wait for the suit before aborting a connection attempt
	boost::posix_time::milliseconds m_initialConnectTimeout;

	/* Reconnection stuff */

	//Schedule a suit reconnection after the reconnect delay
	void scheduleDelayedSuitReconnect();

	//Schedule an immediate suit reconnection attempt
	void scheduleImmediateSuitReconnect();
	
	//Responsible for scheduling a suit reconnection
	void beginReconnectionProcess();

	//Responsible for starting up the monitor & reading process
	void endReconnectionProcess();

	//A timer which allows us to delay reconnects 
	boost::asio::deadline_timer m_suitReconnectionTimer;

	//The delay between port scanning & reconnection attempts
	boost::posix_time::milliseconds m_suitReconnectionTimeout;


};

