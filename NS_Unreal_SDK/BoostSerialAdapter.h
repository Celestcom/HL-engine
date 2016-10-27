#pragma once
#include <memory>
#include "ICommunicationAdapter.h"
#include "boost\asio.hpp"
#include "boost\thread.hpp"
#include "handler_allocator.h"
#include <boost\asio\deadline_timer.hpp>
#include "SuitHardwareInterface.h"
class BoostSerialAdapter : public std::enable_shared_from_this<BoostSerialAdapter>, public virtual ICommunicationAdapter
{
public:
	bool Connect() override;
	void Disconnect() override;
	void Write(uint8_t bytes[], std::size_t length) override;
	void Read() override;
	bool Connect(std::string name);
	std::shared_ptr<CircularBuffer> GetDataStream() override;
	bool IsConnected() const override;
	BoostSerialAdapter(std::shared_ptr<boost::asio::io_service>, std::shared_ptr<SuitHardwareInterface> hardware);
	~BoostSerialAdapter();
private:
	handler_allocator _allocator;
	std::shared_ptr<boost::asio::io_service> _io;
	std::unique_ptr<boost::asio::serial_port> port;
	bool autoConnectPort();
	bool createPort(std::string name);
	uint8_t _data[64];
	void copy_data_to_circularbuff(std::size_t length);
	std::shared_ptr<CircularBuffer> suitDataStream;
	void read_handler(boost::system::error_code ec, std::size_t length);
	boost::asio::deadline_timer _readSuitTimer;
	boost::posix_time::milliseconds _readSuitTimeout = boost::posix_time::milliseconds(100);
	void doSuitRead();
	void suitReadCancel(boost::system::error_code ec);

	boost::asio::deadline_timer _keepaliveTimer;
	boost::posix_time::milliseconds _keepaliveInterval = boost::posix_time::milliseconds(100);
	void doKeepAlivePing(const boost::system::error_code& e);
	std::shared_ptr<SuitHardwareInterface> _hardware;
};

