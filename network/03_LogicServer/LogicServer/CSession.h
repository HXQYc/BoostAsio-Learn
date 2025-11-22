/**
 * @file CSession.h
 * @brief 会话类头文件
 * @details 负责处理单个客户端连接，包括数据收发、消息解析等
 */

#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <queue>
#include <mutex>
#include <memory>
#include "const.h"
#include "MsgNode.h"

class CServer;
class LogicSystem;

/**
 * @class CSession
 * @brief 客户端会话类
 * @details 管理单个客户端连接，负责异步数据收发、消息解析、消息队列管理等
 *          继承自enable_shared_from_this，支持智能指针管理
 */
class CSession: public std::enable_shared_from_this<CSession>
{
public:
	CSession(boost::asio::io_context& io_context, CServer* Server);
	~CSession();

	boost::asio::ip::tcp::socket& GetSocket();
	std::string& GetUuid();
	void Start();

	void Send(char* msg, short max_length, short msgid);
	void Send(std::string msg, short msgid);

	void Close();
	std::shared_ptr<CSession> SharedSelf();

private:
	void HandleRead(const boost::system::error_code& error, size_t bytes_transferred, std::shared_ptr<CSession> shared_self);
	void HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self);

	boost::asio::ip::tcp::socket _socket;					// TCP套接字
	std::string _uuid;										// 会话唯一标识符uuid
	char _data[MAX_LENGTH];									// 接收数据缓冲区
	CServer* _server;										// 所属服务器指针
	bool _b_close;                                          // 是否已关闭标志
	std::queue<std::shared_ptr<SendNode> > _send_que;		// 发送消息队列
	std::mutex _send_lock;									// 发送队列互斥锁
	std::shared_ptr<RecvNode> _recv_msg_node;				// 接收消息节点（存储完整消息）
	bool _b_head_parse;										// 是否已解析消息头标志
	std::shared_ptr<MsgNode> _recv_head_node;				// 接收消息头节点（存储消息头）
};

/**
 * @class LogicNode
 * @brief 逻辑节点类
 * @details 封装会话和接收消息节点，用于投递到逻辑系统处理
 */
class LogicNode {
	friend class LogicSystem;
public:
	/**
	 * @brief 构造函数
	 * @param session 会话对象
	 * @param recvnode 接收消息节点
	 */
	LogicNode(std::shared_ptr<CSession>, std::shared_ptr<RecvNode>);

private:
	std::shared_ptr<CSession> _session;    // 会话对象
	std::shared_ptr<RecvNode> _recvnode;   // 接收消息节点
};

