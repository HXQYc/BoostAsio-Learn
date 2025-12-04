#pragma once
#include <iostream>

class String
{
public:
	String();
	String(const String& otherStr);
	String(const char* otherStr);
	~String();

	String operator+(const String& otherStr) const;		// 字符串拼接
	String operator-(const String& otherStr) const;		// 删除子串
	String& operator=(const String& otherStr);
	String& operator=(const char* str);                  // C字符串赋值

	bool replace(size_t pos, const String& subStr, const String& newStr);	// 替换子串
	int find(const String& subStr) const;				// 查找子串
	int find(const char* subStr) const;                   // 查找子串

private:
	void renewBuffer(size_t requiredSize);				// buffer_size * 2


	char* _buffer;				// 动态缓冲区
	size_t _buffer_size;		// 缓冲区总容量
	size_t _data_length;		// 实际存储长度
	static const size_t INIT_BUFF_SIZE = 128;	// 初始缓冲区大小
};

