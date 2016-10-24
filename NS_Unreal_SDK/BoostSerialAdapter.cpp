#include "BoostSerialAdapter.h"
#include "enumser.h"
#include <iostream>
#include "BlockingReader.h"

bool BoostSerialAdapter::Connect()
{
	return this->autoConnectPort();

}

void BoostSerialAdapter::Disconnect()
{
}

void BoostSerialAdapter::Write(uint8_t bytes[], std::size_t length)
{
	if (this->port && this->port->is_open()) {
		char *chars = reinterpret_cast<char*>(bytes);
		this->port->async_write_some(boost::asio::buffer(bytes, length), 
			[](const boost::system::error_code& error, std::size_t bytes_transferred) {
				if (error) { 
					std::cout << "Couldnt write bytes!" << "\n"; 
				} else { 
				} 
		});
	}
}

void BoostSerialAdapter::Read()
{
	if (this->port->is_open()) {
		auto self(shared_from_this());
		this->port->async_read_some(boost::asio::buffer(_data, 64),
			[this, self](boost::system::error_code ec, std::size_t length) {
			if (!ec && length > 0) {
				self->copy_data_to_circularbuff(length);
			}
			else {
				std::cout << "Error reading bytes!" << std::endl;
			}
		});
	}
	
}
void BoostSerialAdapter::copy_data_to_circularbuff(std::size_t length) {
	for (std::size_t i = 0; i < length; ++i) {
		suitDataStream->push_front(_data[i]);
	}

	std::fill(_data,_data+64, 0);
}



bool BoostSerialAdapter::Connect(std::string name)
{
	return false;
}

std::shared_ptr<CircularBuffer> BoostSerialAdapter::GetDataStream()
{
	return this->suitDataStream;
}

BoostSerialAdapter::BoostSerialAdapter(std::shared_ptr<boost::asio::io_service> io):suitDataStream(std::make_shared<CircularBuffer>(4096)), port(nullptr), _io(io)
{
	std::fill(_data, _data + 64, 0);

}


BoostSerialAdapter::~BoostSerialAdapter()
{
}

bool BoostSerialAdapter::autoConnectPort()
{
	CEnumerateSerial::CPortsArray ports;
	CEnumerateSerial::CNamesArray names;
	if (!CEnumerateSerial::UsingQueryDosDevice(ports)) {
		std::cout << "No ports available." << "\n";
		return false;
	}
	bool foundPort = false;
	for (std::size_t i = 0; i < ports.size(); ++i) {
		std::string strname = "COM" + std::to_string(ports[i]);
		if (this->createPort(strname)) {
			char ping[] = { 0x24, 0x02, 0x02, 0x07, 0xFF, 0xFF, 0x0A };
			boost::asio::write(*(this->port), boost::asio::buffer(ping, 7));
			blocking_reader blocker(*this->port, 10);
			char c;
			if (blocker.read_char(c)) {
				foundPort = true;
				break;
			}


		}
	}
	if (foundPort && this->port) {
		return this->port->is_open();
	}
	else {
		return false;
	}
}

bool BoostSerialAdapter::createPort(std::string name)
{
	this->port = std::unique_ptr<boost::asio::serial_port>(new boost::asio::serial_port(*_io));
	this->port->open(name);
	if (port->is_open()) {
		return true;
	}
	else {
		std::cout << "Tried to use " << name << " but couldn't open it." << "\n";
		return false;
	}
}

