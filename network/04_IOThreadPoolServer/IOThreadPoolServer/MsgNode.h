/**
 * @file MsgNode.h
 * @brief 消息节点类头文件
 * @details 定义了消息节点的基类和派生类，用于封装发送和接收的消息数据
 */

#pragma once
#include <string>
#include "const.h"
#include <iostream>
#include <boost/asio.hpp>

class LogicSystem;

/**
 * @class MsgNode
 * @brief 消息节点基类
 * @details 封装消息数据的基本操作，包括数据缓冲区、当前长度、总长度等
 */
class MsgNode
{
public:
	MsgNode(short max_len) :_total_len(max_len), _cur_len(0)
	{
		_data = new char[_total_len + 1]();
		_data[_total_len] = '\0';
	}
	~MsgNode()
	{
		std::cout << "destruct MsgNode" << std::endl;
		delete[] _data;
	}
	void Clear() {
		::memset(_data, 0, _total_len);
		_cur_len = 0;
	}

	short _cur_len;		// 当前已接收/发送的长度
	short _total_len;	// 消息总长度
	char* _data;		// 消息数据缓冲区
};

/**
 * @class RecvNode
 * @brief 接收消息节点类
 * @details 继承自MsgNode，用于封装接收到的消息，包含消息ID
 */
class RecvNode :public MsgNode {
	friend class LogicSystem;
public:
	RecvNode(short max_len, short msg_id);

private:
	short _msg_id;       // 消息ID
};

/**
 * @class SendNode
 * @brief 发送消息节点类
 * @details 继承自MsgNode，用于封装要发送的消息，包含消息ID和完整的消息头
 */
class SendNode :public MsgNode {
	friend class LogicSystem;
public:
	SendNode(const char* msg, short max_len, short msg_id);

private:
	short _msg_id;       // 消息ID
};

