/**
 * @file LogicSystem.cpp
 * @brief 逻辑系统类实现文件
 * @details 实现业务逻辑处理、消息队列管理等功能
 */

#include "LogicSystem.h"

using namespace std;

/**
 * @brief 构造函数实现
 * @details 初始化停止标志为false，注册回调函数，启动工作线程
 */
LogicSystem::LogicSystem() :_b_stop(false) {
	RegisterCallBacks();
	_worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

/**
 * @brief 析构函数实现
 * @details 设置停止标志，通知工作线程，等待工作线程结束
 */
LogicSystem::~LogicSystem() {
	_b_stop = true;
	_consume.notify_one();
	_worker_thread.join();
}

/**
 * @brief 将消息投递到消息队列
 * @details 线程安全地添加消息到队列，如果队列从空变为非空，通知工作线程处理
 */
void LogicSystem::PostMsgToQue(shared_ptr < LogicNode> msg) {
	std::unique_lock<std::mutex> unique_lk(_mutex);
	_msg_que.push(msg);
	// 如果队列从0变为1，通知工作线程（从等待状态唤醒）
	if (_msg_que.size() == 1) {
		unique_lk.unlock();
		_consume.notify_one();
	}
}

/**
 * @brief 处理消息的工作函数
 * @details 在工作线程中循环运行，从消息队列中取出消息并调用对应的回调函数处理
 *          支持优雅关闭：停止时处理完队列中所有剩余消息
 */
void LogicSystem::DealMsg() {
	for (;;) {
		std::unique_lock<std::mutex> unique_lk(_mutex);
		// 判断队列是否为空，如果为空且未停止，则等待条件变量通知
		while (_msg_que.empty() && !_b_stop) {
			_consume.wait(unique_lk);
		}

		// 判断是否为关闭状态，如果是，处理完所有剩余消息后退出循环
		if (_b_stop) {
			while (!_msg_que.empty()) {
				auto msg_node = _msg_que.front();
				cout << "recv_msg id  is " << msg_node->_recvnode->_msg_id << endl;
				// 查找对应的回调函数
				auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
				if (call_back_iter == _fun_callbacks.end()) {
					// 未找到对应的回调函数，跳过该消息
					_msg_que.pop();
					continue;
				}
				// 调用回调函数处理消息
				call_back_iter->second(
					msg_node->_session, 
					msg_node->_recvnode->_msg_id,
					std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len)
				);
				_msg_que.pop();
			}
			break;
		}

		// 如果没有停止，说明队列中有消息需要处理
		auto msg_node = _msg_que.front();
		cout << "recv_msg id  is " << msg_node->_recvnode->_msg_id << endl;
		// 查找对应的回调函数
		auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
		if (call_back_iter == _fun_callbacks.end()) {
			// 未找到对应的回调函数，跳过该消息
			_msg_que.pop();
			continue;
		}
		// 调用回调函数处理消息
		call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
			std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
		_msg_que.pop();
	}
}

/**
 * @brief 注册所有消息回调函数
 * @details 将消息ID和对应的处理函数绑定，注册到回调映射表中
 */
void LogicSystem::RegisterCallBacks() {
	// 注册MSG_HELLO_WORD消息的处理回调
	_fun_callbacks[MSG_HELLO_WORD] = std::bind(
		&LogicSystem::HelloWordCallBack, this,
		placeholders::_1, 
		placeholders::_2, 
		placeholders::_3
	);
}

/**
 * @brief Hello World消息回调函数
 * @details 解析JSON格式的消息数据，处理业务逻辑，构造回复消息并发送给客户端
 */
void LogicSystem::HelloWordCallBack(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	// 解析JSON消息
	reader.parse(msg_data, root);
	std::cout << "recevie msg id  is " << root["id"].asInt() << " msg data is "
		<< root["data"].asString() << endl;
	// 构造回复消息
	root["data"] = "server has received msg, msg data is " + root["data"].asString();
	std::string return_str = root.toStyledString();
	// 发送回复消息给客户端
	session->Send(return_str, root["id"].asInt());
}
