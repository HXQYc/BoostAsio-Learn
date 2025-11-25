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
#include "AsioIOContextPool.h"

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
	try
	{
		boost::asio::io_context io_c;

		// 设置信号处理，捕获SIGINT（Ctrl+C）和SIGTERM信号
		boost::asio::signal_set signals(io_c, SIGINT, SIGTERM);
		signals.async_wait(
			[&io_c](auto, auto) {
				// 收到信号时停止io上下文，优雅关闭服务器
				io_c.stop();
			}
		);

		// 创建服务器对象，监听10086端口
		CServer s(io_c, 10086);

		// 运行IO上下文的事件循环，处理所有异步操作
		io_c.run();
	}
	catch (std::exception& e) {
		// 捕获并输出异常信息
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