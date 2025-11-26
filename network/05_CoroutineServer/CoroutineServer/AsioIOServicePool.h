#pragma once
#include <vector>
#include <boost/asio.hpp>

class AsioIOServicePool
{
public:
	using IOService = boost::asio::io_context;
	using WorkGuard = boost::asio::executor_work_guard<IOService::executor_type>;
	using WorkGuardPtr = std::unique_ptr<WorkGuard>;

	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;

	// 轮询的方法获取io_context
	boost::asio::io_context& GetIOService();
	void Stop();

	static AsioIOServicePool& GetInstance();

private:
	AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());
	std::vector<IOService> _ioServices;
	std::vector<WorkGuardPtr> _workGuards;
	std::vector<std::thread> _threads;
	std::size_t _nextIOService;
};

