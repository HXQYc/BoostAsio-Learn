#include "CSession.h"
#include "CServer.h"
#include <iostream>

// 构造函数：初始化会话
CSession::CSession(boost::asio::io_context& io_context, CServer* server) :
	_socket(io_context), _server(server), _b_close(false), _b_head_parse(false)
{
	// 生成唯一UUID
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);

	// 创建头部节点
	_recv_head_node = std::make_shared<MsgNode>(HEAD_LENGTH);
}

// 析构函数
CSession::~CSession() {
	std::cout << "~CSession destruct" << std::endl;
}

// --------------------- 对外接口 ---------------------
// 获取socket引用
boost::asio::ip::tcp::socket& CSession::GetSocket()
{
	return _socket;
}

// 获取uuid
std::string& CSession::GetUuid() {
	return _uuid;
}

// 获取shared_ptr自身
std::shared_ptr<CSession> CSession::SharedSelf() {
	return shared_from_this();
}


// --------------------- 直接与会话相关 ---------------------
// 启动会话（开始异步读）
void CSession::Start()
{
	// 清空接收缓冲区
	:: memset(_data, 0, MAX_LENGTH); 

	// 注册读回调函数
	_socket.async_read_some(
		boost::asio::buffer(_data, MAX_LENGTH),
		std::bind(
			&CSession::HandleRead,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			SharedSelf()
		)
	);
	/*// lambda表达式风格
	_socket.async_read_some(
		boost::asio::buffer(_data, MAX_LENGTH),
		[self = SharedSelf()](boost::system::error_code ec, size_t bytes) {
			// 直接捕获需要的参数
			self->HandleRead(ec, bytes, self);
		}
	);
	*/
}

// 发送消息
void CSession::Send(char* msg, int max_length)
{
	std::lock_guard<std::mutex> lock(_send_lock); // 加锁保护发送队列

	// 发送消息时，首先要检查发送队列是否已满
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE)
	{
		// 满则不发送
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}

	// 将消息加入发送队列
	_send_que.push(std::make_shared<MsgNode>(msg, max_length));

	// 队列中原本有消息，直接返回。即等待当前发送完成
	if (send_que_size > 0)
	{
		return;
	}

	// 队列为空，立即开始发送
	auto& msgnode = _send_que.front();
	boost::asio::async_write(
		_socket, 
		boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(
			&CSession::HandleWrite, 
			this, 
			std::placeholders::_1, 
			SharedSelf()
		)
	);
}

// 关闭会话
void CSession::Close()
{
	_socket.close();
	_b_close = true;
}

// --------------------- private ---------------------
// 异步读回调
void CSession::HandleRead(const boost::system::error_code& error, size_t bytes_transferred, std::shared_ptr<CSession> shared_self)
{
	/*
	* error 错误码
	* bytes_transferred 实际读取的字节数
	* shared_self 生命周期管理
	*/

	if (!error)
	{
		int copy_len = 0; // 已从_data缓冲区复制的数据长度

		// 循环处理所有接收到的数据（可能包括多个消息或者部分消息）
		while (bytes_transferred > 0)
		{
			// ==================== 阶段1：处理消息头部 ====================
			if (!_b_head_parse)
			{
				// 情况1：接收的数据 + 已缓存的头部数据仍不足完整的头部大小
				if (bytes_transferred + _recv_head_node->_cur_len < HEAD_LENGTH)
				{
					// 将本次接收的数据追加到头部缓存中
					memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);	// 把已经读到_data中的数据放入_recv_head_node中
					// 更新已接收的头部数据长度
					_recv_head_node->_cur_len += bytes_transferred;	

					// 重置_data，清空接收缓存区
					::memset(_data, 0, MAX_LENGTH);	

					// 继续读剩余数据
					_socket.async_read_some(
						boost::asio::buffer(_data, MAX_LENGTH),
						std::bind(
							&CSession::HandleRead,
							this,
							std::placeholders::_1,
							std::placeholders::_2,
							shared_self
						)
					);

					return; //退出当前处理，等待更多数据
				}

				// 情况2：接收的数据足够解析完整的消息头部
				// 计算头部剩余需要的数据长度
				int head_remain = HEAD_LENGTH - _recv_head_node->_cur_len;

				// 复制剩余头部数据到头部节点
				memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
				// 更新已处理数据位置和剩余数据长度
				copy_len += head_remain;
				bytes_transferred -= head_remain;

				// 从头部数据中提取消息体长度（前2字节）
				short data_len = 0;
				memcpy(&data_len, _recv_head_node->_data, HEAD_LENGTH);

				// 网络字节序转主机字节序
				data_len = boost::asio::detail::socket_ops::network_to_host_short(data_len);
				std::cout << "data_len is " << data_len << std::endl;

				// 安全检查：检查消息长度是否合法（防止恶意数据）
				if (data_len > MAX_LENGTH) {
					std::cout << "invalid data length is " << data_len << std::endl;
					_server->ClearSession(_uuid); // 清除非法会话
					return;
				}

				// 创建消息节点，准备接收消息体数据
				_recv_msg_node = std::make_shared<MsgNode>(data_len);

				// 重置状态，准备进入下一次循环来处理消息体
				_b_head_parse = true;        // 重置头部解析标志
				continue;
			}

			// ==================== 阶段2：处理消息体（之前已解析头部但数据不完整） ====================

			// 计算当前消息体还有多少数据需要接收
			int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;

			// 情况1：接收的数据不足以完成当前消息体
			if (bytes_transferred < remain_msg)
			{
				// 将本次接收的数据追加到消息体节点
				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
				_recv_msg_node->_cur_len += bytes_transferred;

				// 清空接收缓冲区
				::memset(_data, 0, MAX_LENGTH);

				// 继续异步读取剩余的消息体数据
				_socket.async_read_some(
					boost::asio::buffer(_data, MAX_LENGTH),
					std::bind(
						&CSession::HandleRead, 
						this, 
						std::placeholders::_1, 
						std::placeholders::_2, 
						shared_self
					)
				);
				return; // 退出当前处理，等待更多数据
			}

			// 情况2：接收的数据足够完成当前消息体
			// 复制剩余需要的消息体数据
			memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
			_recv_msg_node->_cur_len += remain_msg;

			// 更新处理位置和剩余数据长度
			bytes_transferred -= remain_msg;
			copy_len += remain_msg;

			// 添加字符串结束符
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			std::cout << "receive data is " << _recv_msg_node->_data << std::endl;

			// 回显接收到的完整消息
			Send(_recv_msg_node->_data, _recv_msg_node->_total_len);

			// 重置状态，准备处理下一条消息
			_b_head_parse = false;        // 重置头部解析标志
			_recv_head_node->Clear();     // 清空头部节点缓存

			// 如果当前批次数据已全部处理完成
			if (bytes_transferred <= 0) {
				::memset(_data, 0, MAX_LENGTH); // 清空接收缓冲区

				// 继续异步读取下一条消息
				_socket.async_read_some(
					boost::asio::buffer(_data, MAX_LENGTH),
					std::bind(&CSession::HandleRead, 
						this, 
						std::placeholders::_1, 
						std::placeholders::_2, 
						shared_self
					)
				);

				return;
			}

			continue; // 继续处理当前批次中的剩余数据（可能包含下一条消息）
		}
	}
	else
	{
		// 读取出错，关闭连接并清理会话
		std::cout << "handle read failed, error is " << error.what() << std::endl;
		Close();
		_server->ClearSession(_uuid);
	}
}
// 异步写回调
void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self)
{
	if (!error)
	{
		// 当执行到这里时，数据已经100%发送完成了！
	   // async_write 保证：要么全部发送成功，要么出错

		std::lock_guard<std::mutex> lock(_send_lock);

		// 打印发送的数据（跳过头部）
		std::cout << "send data: " << _send_que.front()->_data + HEAD_LENGTH << std::endl;
		_send_que.pop(); // 移除已确认发送的消息

		// 继续处理队列中的数据
		// （可能在asyncwrite过程中，用户调用send填充到队列
		if (!_send_que.empty()) {
			auto& msgnode = _send_que.front();
			boost::asio::async_write(
				_socket,
				boost::asio::buffer(msgnode->_data, msgnode->_total_len),
				std::bind(
					&CSession::HandleWrite,
					this, std::placeholders::_1,
					shared_self
				)
			);
		}
	}
	else
	{
		// 发送出错，关闭连接并清理会话
		std::cout << "handle write failed, error is " << error.what() << std::endl;
		Close();
		_server->ClearSession(_uuid);
	}
}