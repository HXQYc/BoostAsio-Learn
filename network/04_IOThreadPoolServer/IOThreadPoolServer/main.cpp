/**
 * @file day13-LogicServer.cpp
 * @brief 逻辑服务器主程序入口
 * @details 启动TCP服务器，监听客户端连接，处理信号以优雅关闭
 */

#include <iostream>
#include "CServer.h"
#include "Singleton.h"
#include "LogicSystem.h"
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioThreadPool.h"

bool bstop = false;					// 停止标志
std::condition_variable cond_quit;	// 退出条件变量
std::mutex mutex_quit;				// 退出互斥锁

/**
 * @brief 主函数
 * @details 创建IO上下文，设置信号处理，启动服务器并运行事件循环
 * @return 程序退出码
 */
int main()
{
	try {
		auto pool = AsioThreadPool::GetInstance();
		boost::asio::io_context io_context;
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([pool, &io_context](auto, auto) {
			io_context.stop();
			pool->Stop();
			});

		CServer s(pool->GetIOService(), 10086);

		//˳io_context
		io_context.run();
		std::cout << "server exited " << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}

// C风格信号处理
//bool bstop = false;
//std::condition_variable cond_quit;
//std::mutex mutex_quit;
//void sig_handler(int sig)
//{
//	if (sig == SIGINT || sig == SIGTERM)
//	{
//		std::unique_lock<std::mutex>  lock_quit(mutex_quit);
//		bstop = true;
//		lock_quit.unlock();
//		cond_quit.notify_one();
//	}
//}
//
//int main()
//{
//	try {
//		boost::asio::io_context  io_context;
//		std::thread  net_work_thread([&io_context] {
//			CServer s(io_context, 10086);
//			io_context.run();
//			});
//		signal(SIGINT, sig_handler);
//		while (!bstop) {
//			std::unique_lock<std::mutex>  lock_quit(mutex_quit);
//			cond_quit.wait(lock_quit);
//		}
//		io_context.stop();
//		net_work_thread.join();
//
//	}
//	catch (std::exception& e) {
//		std::cerr << "Exception: " << e.what() << endl;
//	}
//
//}