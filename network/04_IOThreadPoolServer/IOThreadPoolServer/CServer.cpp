/**
 * @file CServer.cpp
 * @brief 服务器类实现文件
 * @details 实现TCP服务器的功能，包括接受连接、管理会话等
 */

#include "CServer.h"
#include <iostream>



CServer::CServer(boost::asio::io_context& io_context, short port) :_io_context(io_context), _port(port),
_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	std::cout << "Server start success, listen on port : " << _port << std::endl;
	StartAccept();
}

void CServer::HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error) {
	if (!error) {
		// 连接成功，启动会话并保存到映射表
		new_session->Start();
		_sessions.insert(make_pair(new_session->GetUuid(), new_session));
	}
	else {
		// 连接失败，输出错误信息
		std::cout << "session accept failed, error is " << error.what() << std::endl;
	}

	// 继续接受下一个客户端连接
	StartAccept();
}

void CServer::StartAccept() {


	// 创建新的会话对象
	std::shared_ptr<CSession> new_session = std::make_shared<CSession>(_io_context, this);
	// 异步接受连接，当有新连接时调用HandleAccept回调
	_acceptor.async_accept(new_session->GetSocket(), std::bind(&CServer::HandleAccept, this, new_session, std::placeholders::_1));
}

void CServer::ClearSession(std::string uuid) {
	_sessions.erase(uuid);
}
