#pragma once

class IoService
{
public:
	IoService();
	~IoService();
	std::shared_ptr<boost::asio::io_service> GetIOService();
	void RestartIOService();
	bool Start();
	bool Stop();
private:
	std::unique_ptr<std::thread> _thread;
	std::shared_ptr<boost::asio::io_service> _io;
	std::unique_ptr<boost::asio::io_service::work> _work;
	std::mutex mutex;
};

