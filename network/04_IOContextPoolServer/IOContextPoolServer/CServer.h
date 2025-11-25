/**
 * @file CServer.h
 * @brief 服务器类头文件
 * @details 负责管理TCP服务器，接受客户端连接，维护所有会话连接
 */

#pragma once
#include <boost/asio.hpp>
#include "CSession.h"
#include <memory.h>
#include <map>

 /**
  * @class CServer
  * @brief TCP服务器类
  * @details 使用Boost.Asio实现异步TCP服务器，负责监听端口、接受客户端连接、
  *          管理所有客户端会话，并在会话断开时清理资源
  */
class CServer
{
public:
	CServer(boost::asio::io_context& io_context, short port);	// 构造
	void ClearSession(std::string uuid);						// 清除指定uuid的会话

private:
	void HandleAccept(std::shared_ptr<CSession>, const boost::system::error_code& error);	// 处理新客户端连接的回调函数
	void StartAccept();		// 异步接受客户端连接

	boost::asio::io_context& _io_context;	// IO上下文引用
	short _port;                            // 服务器监听端口
	boost::asio::ip::tcp::acceptor _acceptor;					 // TCP接受器，用于接受客户端连接
	std::map<std::string, std::shared_ptr<CSession>> _sessions;  // 会话映射表，key为UUID，value为会话对象
};

