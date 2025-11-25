#pragma once
#include <boost/asio.hpp>
#include "Singleton.h"

class AsioThreadPool:public Singleton<AsioThreadPool>
{
public:
	using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

	friend Singleton<AsioThreadPool>;
	~AsioThreadPool() {}
	AsioThreadPool(const AsioThreadPool&) = delete;
	AsioThreadPool& operator=(const AsioThreadPool&) = delete;

	boost::asio::io_context& GetIOService();
	void Stop();
private:
	AsioThreadPool(int threadNum = std::thread::hardware_concurrency());
	boost::asio::io_context _service;
	std::unique_ptr<WorkGuard> _workGuard;
	std::vector<std::thread> _threads;
};

