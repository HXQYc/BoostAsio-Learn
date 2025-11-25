/**
 * @file LogicSystem.h
 * @brief 逻辑系统类头文件
 * @details 负责处理业务逻辑，使用单例模式和生产者-消费者模式处理消息
 */

#pragma once
#include "Singleton.h"
#include <queue>
#include <thread>
#include "CSession.h"
#include <queue>
#include <map>
#include <functional>
#include "const.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>


 /**
  * @typedef FunCallBack
  * @brief 消息回调函数类型定义
  * @details 处理消息的回调函数，参数包括会话对象、消息ID和消息数据
  */
typedef std::function<void(std::shared_ptr<CSession>, const short& msg_id, const std::string& msg_data)> FunCallBack;

/**
 * @class LogicSystem
 * @brief 逻辑系统类
 * @details 单例模式，负责处理所有业务逻辑消息
 *          使用独立的工作线程处理消息队列，通过回调函数映射表处理不同类型的消息
 */
class LogicSystem: public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();
	void PostMsgToQue(std::shared_ptr <LogicNode>msg);

private:
	/**
	 * @brief 私有构造函数
	 * @details 注册所有消息回调函数，启动工作线程
	 */
	LogicSystem();

	/**
	 * @brief 处理消息的工作函数
	 * @details 在工作线程中运行，从消息队列中取出消息并调用对应的回调函数处理
	 */
	void DealMsg();

	/**
	 * @brief 注册所有消息回调函数
	 * @details 将消息ID和对应的处理函数注册到回调映射表中
	 */
	void RegisterCallBacks();

	/**
	* @brief Hello World消息回调函数
	 * @param session 会话对象
	 * @param msg_id 消息ID
	 * @param msg_data 消息数据（JSON格式字符串）
	 * @details 解析JSON消息，处理业务逻辑，并回复客户端
	 */
	void HelloWordCallBack(std::shared_ptr<CSession>, const short& msg_id, const std::string& msg_data);

	std::thread _worker_thread;								// 工作线程
	std::queue<std::shared_ptr<LogicNode>> _msg_que;		// 消息队列
	std::mutex _mutex;										// 消息队列互斥锁
	std::condition_variable _consume;						// 条件变量，用于通知工作线程
	bool _b_stop;											// 停止标志
	std::map<short, FunCallBack> _fun_callbacks;			// 消息ID到回调函数的映射表
};

