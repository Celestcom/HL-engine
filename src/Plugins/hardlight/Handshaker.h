#pragma once
#include <boost/asio/serial_port.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
class Handshaker {
public:
	enum class Status {
		Unknown,
		Closed,
		Open,
		Connected,
		Disconnected,
		Unreadable,
		Unwritable,
		TimedOutReading,
		TimedOutWriting,
		BadReturnPing,
	};
	void start_handshake();
	Handshaker(std::string name, boost::asio::io_service& io);
private:
	virtual void setup_port_options(boost::asio::serial_port& port) = 0;
	virtual const uint8_t* ping_data() const = 0;
	virtual std::size_t ping_data_length() const = 0;
	virtual bool is_good_response(const uint8_t* data, unsigned int length) const = 0;

	virtual const boost::posix_time::time_duration& read_timeout() const = 0;
	virtual const boost::posix_time::time_duration& write_timeout() const = 0;

	bool m_protocolFinished;
	Status m_status;
	std::string m_name;
	std::unique_ptr<boost::asio::serial_port> m_port;
	boost::asio::io_service& m_io;

	boost::asio::deadline_timer m_writeTimer;
	boost::asio::deadline_timer m_readTimer;

	void stop();

	void async_open_port();
	void async_ping_port();
	void start_read();
	void read_handler(const boost::system::error_code& ec, std::size_t bytes_transferred);
	void write_handler(const boost::system::error_code& ec, std::size_t bytes_transferred);
	void check_write_deadline(const boost::system::error_code& ec);
	void check_read_deadline(const boost::system::error_code& ec);

	const static unsigned int INCOMING_DATA_BUFFER_SIZE = 128;

	//our incoming data buffer 
	uint8_t m_data[INCOMING_DATA_BUFFER_SIZE];


};