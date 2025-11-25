/**
 * @file CSession.cpp
 * @brief 会话类实现文件
 * @details 实现客户端会话的数据收发、消息解析等功能
 */

#include "CSession.h"
#include "CServer.h"
#include <iostream>
#include <sstream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "LogicSystem.h"

CSession::CSession(boost::asio::io_context& io_context, CServer* server) :
	_socket(io_context), _server(server), _b_close(false), _b_head_parse(false),
	_strand(io_context.get_executor())//共享同一个执行器，达到串行的效果

{
	// 生成随机UUID作为会话唯一标识
	boost::uuids::uuid  a_uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);
	// 创建接收消息头节点，大小为消息头总长度
	_recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

CSession::~CSession() {
	std::cout << "~CSession destruct" << std::endl;
}

boost::asio::ip::tcp::socket& CSession::GetSocket() {
	return _socket;
}

std::string& CSession::GetUuid() {
	return _uuid;
}

void CSession::Start() {
	::memset(_data, 0, MAX_LENGTH);
	// 异步读取数据，读取完成后调用HandleRead回调
	//_socket.async_read_some(
	//	boost::asio::buffer(_data, MAX_LENGTH), 
	//	std::bind(
	//		&CSession::HandleRead, 
	//		this,
	//		std::placeholders::_1, 
	//		std::placeholders::_2, 
	//		SharedSelf()
	//	)
	//);

	// 通过strand来调用：生成一个新的执行器，
	_socket.async_read_some(
		boost::asio::buffer(_data, MAX_LENGTH),
		boost::asio::bind_executor(
			_strand,
			std::bind(
				&CSession::HandleRead,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				SharedSelf()
			)
		)
	);
}

void CSession::Send(std::string msg, short msgid) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();
	// 检查发送队列是否已满
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}

	// 将消息加入发送队列
	_send_que.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
	// 如果队列中已有消息，说明正在发送，直接返回（会在HandleWrite中继续发送）
	if (send_que_size > 0) {
		return;
	}
	// 队列为空，立即发送
	auto& msgnode = _send_que.front();
	//boost::asio::async_write(
	//	_socket, 
	//	boost::asio::buffer(msgnode->_data, msgnode->_total_len),
	//	std::bind(
	//		&CSession::HandleWrite, 
	//		this, 
	//		std::placeholders::_1, 
	//		SharedSelf()
	//	)
	//);
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		boost::asio::bind_executor(_strand,
			std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()))
	);
}

void CSession::Send(char* msg, short max_length, short msgid) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();
	// 检查发送队列是否已满
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}

	// 将消息加入发送队列
	_send_que.push(std::make_shared<SendNode>(msg, max_length, msgid));
	// 如果队列中已有消息，说明正在发送，直接返回
	if (send_que_size > 0) {
		return;
	}
	// 队列为空，立即发送
	auto& msgnode = _send_que.front();
	/*boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));*/
	// strand
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		boost::asio::bind_executor(_strand, 
			std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()))
	);
}

void CSession::Close() {
	_socket.close();
	_b_close = true;
}

std::shared_ptr<CSession>CSession::SharedSelf() {
	return shared_from_this();
}

/**
 * @brief 处理写入数据的回调函数
 * @details 发送完成后，从队列中移除已发送的消息，继续发送下一条消息
 */
void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self) {
	// 异常处理保护
	try {
		if (!error) {
			// 发送成功，移除已发送的消息
			std::lock_guard<std::mutex> lock(_send_lock);
			//cout << "send data " << _send_que.front()->_data+HEAD_LENGTH << endl;
			_send_que.pop();
			// 如果队列中还有消息，继续发送
			if (!_send_que.empty()) {
				auto& msgnode = _send_que.front();
				/*boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
					std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_self));*/
				boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
					boost::asio::bind_executor(_strand,
						std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf())
					)
				);
			}
		}
		else {
			// 发送失败，关闭连接并清理会话
			std::cout << "handle write failed, error is " << error.what() << std::endl;
			Close();
			_server->ClearSession(_uuid);
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception code : " << e.what() << std::endl;
	}

}

/**
 * @brief 处理读取数据的回调函数
 * @details 解析消息头和数据，支持分片接收，将完整消息投递到逻辑系统
 *          消息格式：消息头(4字节) = 消息ID(2字节) + 数据长度(2字节) + 消息数据
 */
void CSession::HandleRead(const boost::system::error_code& error, size_t  bytes_transferred, std::shared_ptr<CSession> shared_self) {
	try {
		if (!error) {
			// 已接收到的数据长度
			int copy_len = 0;
			// 循环处理接收到的数据，可能包含多条消息
			while (bytes_transferred > 0) {
				// 如果还没有解析消息头
				if (!_b_head_parse) {
					// 接收到的数据加上已接收的头数据还不够一个完整的消息头
					if (bytes_transferred + _recv_head_node->_cur_len < HEAD_TOTAL_LEN) {
						// 将数据复制到消息头节点，继续接收
						memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
						_recv_head_node->_cur_len += bytes_transferred;
						::memset(_data, 0, MAX_LENGTH);
						//_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						//	std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));

						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
							boost::asio::bind_executor(
								_strand,
								std::bind(
									&CSession::HandleRead,
									this,
									std::placeholders::_1,
									std::placeholders::_2,
									SharedSelf()
								)));

						return;
					}
					// 接收到的数据已经足够或超过消息头大小
					// 计算消息头剩余未复制的长度
					int head_remain = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
					memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
					// 更新已处理的data长度和剩余未处理的长度
					copy_len += head_remain;
					bytes_transferred -= head_remain;
					// 从消息头中提取MSGID（前2字节）
					short msg_id = 0;
					memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
					// 网络字节序转换为主机字节序
					msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
					std::cout << "msg_id is " << msg_id << std::endl;
					// 验证消息ID是否合法
					if (msg_id > MAX_LENGTH) {
						std::cout << "invalid msg_id is " << msg_id << std::endl;
						_server->ClearSession(_uuid);
						return;
					}
					// 从消息头中提取数据长度（后2字节）
					short msg_len = 0;
					memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
					// 网络字节序转换为主机字节序
					msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
					std::cout << "msg_len is " << msg_len << std::endl;
					// 验证数据长度是否合法
					if (msg_len > MAX_LENGTH) {
						std::cout << "invalid data length is " << msg_len << std::endl;
						_server->ClearSession(_uuid);
						return;
					}

					// 创建接收消息节点
					_recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);

					// 消息的长度小于消息头规定的长度，说明数据还未接收完整，先将已接收的数据放到接收节点中
					if (bytes_transferred < msg_len) {
						memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
						_recv_msg_node->_cur_len += bytes_transferred;
						::memset(_data, 0, MAX_LENGTH);
						//_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						//	std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
							boost::asio::bind_executor(_strand, std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self)));
						// 消息头已解析完成
						_b_head_parse = true;
						return;
					}

					// 数据已接收完整，复制到接收节点
					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, msg_len);
					_recv_msg_node->_cur_len += msg_len;
					copy_len += msg_len;
					bytes_transferred -= msg_len;
					_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
					//cout << "receive data is " << _recv_msg_node->_data << endl;
					// 此处将消息投递到逻辑系统处理
					LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));

					// 继续查询剩余未处理的数据
					_b_head_parse = false;
					_recv_head_node->Clear();
					if (bytes_transferred <= 0) {
						::memset(_data, 0, MAX_LENGTH);
						//_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						//	std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
							boost::asio::bind_executor(_strand, std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self)));

						return;
					}
					continue;
				}

				// 已经解析过消息头，正在接收上次未接收完整的消息数据
				// 计算接收到的数据是否足够剩余未接收的数据
				int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
				if (bytes_transferred < remain_msg) {
					// 数据还不够，继续接收
					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
					_recv_msg_node->_cur_len += bytes_transferred;
					::memset(_data, 0, MAX_LENGTH);
					//_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
					//	std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						boost::asio::bind_executor(_strand, std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self)));
					return;
				}
				// 数据已足够，复制剩余数据
				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
				_recv_msg_node->_cur_len += remain_msg;
				bytes_transferred -= remain_msg;
				copy_len += remain_msg;
				_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
				//cout << "receive data is " << _recv_msg_node->_data << endl;
				// 此处将消息投递到逻辑系统处理
				LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));

				// 继续查询剩余未处理的数据
				_b_head_parse = false;
				_recv_head_node->Clear();
				if (bytes_transferred <= 0) {
					::memset(_data, 0, MAX_LENGTH);
					//_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
					//	std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
						boost::asio::bind_executor(_strand, std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self)));

					return;
				}
				continue;
			}
		}
		else {
			// 读取失败，关闭连接并清理会话
			std::cout << "handle read failed, error is " << error.what() << std::endl;
			Close();
			_server->ClearSession(_uuid);
		}
	}
	catch (std::exception& e) {
		std::cout << "Exception code is " << e.what() << std::endl;
	}
}

/**
 * @brief LogicNode构造函数实现
 */
LogicNode::LogicNode(std::shared_ptr<CSession>  session, std::shared_ptr<RecvNode> recvnode) 
	:_session(session), _recvnode(recvnode) {

}
