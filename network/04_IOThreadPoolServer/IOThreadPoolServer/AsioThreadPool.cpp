#include "AsioThreadPool.h"

AsioThreadPool::AsioThreadPool(int threadNum)
	:_workGuard(std::make_unique<WorkGuard>(
		boost::asio::make_work_guard(_service)
	))
{
	for (int i = 0; i < threadNum; ++i)
	{
		_threads.emplace_back([this]() {
			_service.run();
			});
	}
}

boost::asio::io_context& AsioThreadPool::GetIOService()
{
	return _service;
}

void AsioThreadPool::Stop()
{
	_workGuard.reset();
	_service.stop();
	for (auto& t : _threads) {
		t.join();
	}
	std::cout << "AsioThreadPool::Stop" << std::endl;
}