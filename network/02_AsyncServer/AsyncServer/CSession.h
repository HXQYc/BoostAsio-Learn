#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <memory>
#include <queue>
#include <mutex>
#include "const.h"

class CServer;

// 消息节点
class MsgNode
{
	friend class CSession;
public:
	// 创建带数据的消息节点
	MsgNode(char* msg, short max_len) :_total_len(max_len + HEAD_LENGTH), _cur_len(0)
	{
		_data = new char[_total_len + 1]();		// 加括号后会自动初始化为'\0'
		// 转为网络字节序
		int max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
		memcpy(_data, &max_len_host, HEAD_LENGTH); // 前两字节赋值为网络字节序的包长度。
		memcpy(_data + HEAD_LENGTH, msg, max_len);
		_data[_total_len] = '\0';
	}

	// 创建空消息节点，用于接收头部
	MsgNode(short max_len) :_total_len(max_len), _cur_len(0)
	{
		_data = new char[_total_len + 1]();
	}

	~MsgNode()
	{
		delete[] _data;
	}

	void Clear()
	{
		::memset(_data, 0, _total_len);
		_cur_len = 0;
	}

private:
	short _cur_len;
	short _total_len;
	char* _data;

};

// tcp会话
class CSession: public std::enable_shared_from_this<CSession>
{
public:
	CSession(boost::asio::io_context& io_context, CServer* server);
	~CSession();

	boost::asio::ip::tcp::socket& GetSocket();		// 获取socket引用
	std::string& GetUuid();							// 获取会话uuid
	std::shared_ptr<CSession> SharedSelf();

	void Start();									// 启动会话（开始异步读）
	void Send(char* msg, int Max_length);			// 发送消息
	void Close();									// 关闭会话

private:
	// 异步读回调
	void HandleRead(const boost::system::error_code& error, size_t bytes_transferred, std::shared_ptr<CSession> shared_self);
	// 异步写回调
	void HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self);
	
	boost::asio::ip::tcp::socket _socket;			// TCP socket 
	std::string _uuid;								// 会话唯一标识
	char _data[MAX_LENGTH];							// 接收缓冲区
	CServer* _server;								// 指向服务器server的指针
	bool _b_close;									// 会话是否关闭的标志
	std::queue<std::shared_ptr<MsgNode>> _send_que;	// 发送消息队列
	std::mutex _send_lock;							// 发送队列互斥锁

	// 收到的消息结构
	std::shared_ptr<MsgNode> _recv_msg_node;		// 当前接收的消息节点
	bool _b_head_parse;								// 头部是否已解析

	// 收到的头部结构
	std::shared_ptr<MsgNode> _recv_head_node;
};

