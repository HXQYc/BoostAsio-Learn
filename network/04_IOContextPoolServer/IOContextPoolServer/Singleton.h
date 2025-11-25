/**
 * @file Singleton.h
 * @brief 单例模式模板类头文件
 * @details 提供线程安全的单例模式实现，使用std::call_once确保只创建一次实例
 */

#pragma once
#include <memory>
#include <mutex>
#include <iostream>

 /**
  * @class Singleton
  * @brief 单例模式模板类
  * @details 使用模板实现，任何继承自Singleton的类都可以获得单例功能
  *          线程安全，使用std::call_once确保只创建一次实例
  * @tparam T 要创建单例的类型
  */
template <typename T>
class Singleton
{
protected:
	Singleton() = default;
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator=(const Singleton<T>&) = delete;

	static std::shared_ptr<T> _instance;	// 单例实例指针

public:
	/**
	 * @brief 获取单例实例
	 * @return 指向单例对象的shared_ptr
	 * @details 使用std::call_once确保线程安全，只创建一次实例
	*/
	static std::shared_ptr<T> GetInstance()
	{
		static std::once_flag s_flag;
		std::call_once(
			s_flag,
			[&]() {
				_instance = std::shared_ptr<T>(new T);
			}
		);

		return _instance;
	}
	/**
	 * @brief 打印实例地址（用于调试）
	 * @details 输出单例对象的内存地址
	 */
	void PrintAddress() {
		std::cout << _instance.get() << std::endl;
	}

	/**
	 * @brief 析构函数
	 */
	~Singleton() {
		std::cout << "this is singleton destruct" << std::endl;
	}
};

// 静态成员变量初始化
template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;
