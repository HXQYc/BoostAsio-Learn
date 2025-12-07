#include "String.h"

String::String()
	:_buffer(nullptr), _buffer_size(0), _data_length(0)
{
	_buffer = new char[INIT_BUFF_SIZE];
	_buffer_size = INIT_BUFF_SIZE;
	_buffer[0] = '\0';
}

String::String(size_t len)
	:_buffer(nullptr), _buffer_size(0), _data_length(len)
{
	_buffer_size = INIT_BUFF_SIZE;
	size_t size = _buffer_size;
	while (size < len) size *= 2;

	_buffer = new char[size];
	_buffer_size = size;
	_buffer[0] = '\0';
}

String::String(const String& otherStr)
	:_buffer(nullptr), _buffer_size(0), _data_length(otherStr._data_length)
{
	_buffer_size = otherStr._buffer_size;
	_buffer = new char[_buffer_size];
	memcpy(_buffer, otherStr._buffer, _data_length);
	_buffer[_data_length] = '\0';
}

String::String(const char* otherStr)
	:_buffer(nullptr), _buffer_size(0), _data_length(0)
{
	if (!otherStr)
	{
		_buffer = new char[INIT_BUFF_SIZE];
		_buffer_size = INIT_BUFF_SIZE;
		_buffer[0] = '\0';
		return;
	}

	size_t len = 0;
	while (otherStr[len] != '\0') len++;
	_data_length = len;
	
	_buffer_size = _data_length < INIT_BUFF_SIZE ? INIT_BUFF_SIZE : _data_length;
	_buffer = new char[_buffer_size];

	memcpy(_buffer, otherStr, _data_length);
	_buffer[len] = '\0';

}

// 赋值运算符重载
String& String::operator=(const String& otherStr)
{
	delete[] _buffer;
	_buffer_size = otherStr._buffer_size;
	_buffer = new char[_buffer_size];
	_data_length = otherStr._data_length;
	memcpy(_buffer, otherStr._buffer, _data_length);
	_buffer[_data_length] = '\0';

	return *this;
}

// C字符串赋值
String& String::operator=(const char* otherStr)
{
	if (!otherStr)
	{
		_buffer = new char[INIT_BUFF_SIZE];
		_buffer_size = INIT_BUFF_SIZE;
		_data_length = 0;
		_buffer[0] = '\0';
		return *this;
	}

	delete[] _buffer;
	size_t len = 0;
	while (otherStr[len] != '\0') len++;
	_data_length = len;

	_buffer_size = _data_length < INIT_BUFF_SIZE ? INIT_BUFF_SIZE : _data_length;
	_buffer = new char[_buffer_size];

	memcpy(_buffer, otherStr, _data_length);
	_buffer[len] = '\0';

	return *this;
}

String::~String()
{
	delete[] _buffer;
	_buffer = nullptr;
}

// 左移运算符重载，用于输出 String 对象
std::ostream& operator<<(std::ostream& os, const String& str)
{
	if (str._buffer)
	{
		// 直接输出 _buffer 的内容
		// _buffer 已经以 '\0' 结尾，所以可以直接作为C字符串输出
		os << str._buffer;
	}
	else
	{
		os << "[null]";
	}
	return os;
}

//// 字符串拼接
//String String::operator+(const String& otherStr) const		
//{
//	size_t newLen = _data_length + otherStr._data_length;
//	String newStr(newLen);
//
//	memcpy(newStr._buffer, _buffer, _data_length);
//	memcpy(newStr._buffer + _data_length, otherStr._buffer, otherStr._data_length);
//	newStr._buffer[newLen] = '\0';
//
//	return newStr;
//}
//
//// 删除子串
//String String::operator-(const String& subStr) const		
//{
//	int pos = find(subStr);
//	if (pos == -1)	return *this;
//
//	size_t new_len = _data_length - subStr._data_length;
//	String newStr(new_len);
//	memcpy(newStr._buffer, _buffer, pos);
//	memcpy(newStr._buffer + pos, _buffer + pos + subStr._data_length, _data_length - pos - subStr._data_length);
//	newStr._data_length = new_len;
//
//	return newStr;
//}
 
String& String::operator+(const String& otherStr)// 字符串拼接
{
	size_t newLen = _data_length + otherStr._data_length;
	if (newLen >= _buffer_size)	renewBuffer(newLen + 1);
	memcpy(_buffer + _data_length, otherStr._buffer, otherStr._data_length);

	_buffer[newLen] = '\0';
	_data_length = newLen;

	return *this;
}
String& String::operator-(const String& otherStr)// 删除子串
{
	// 1. 查找子串位置
	int pos = this->find(otherStr);
	if (pos == -1)
	{
		// 没找到子串，直接返回原字符串
		return *this;
	}

	// 2. 计算删除后的新长度
	size_t new_len = _data_length - otherStr._data_length;

	// 3. 如果删除的是末尾，直接截断
	if (pos + otherStr._data_length == _data_length)
	{
		_buffer[pos] = '\0';
		_data_length = new_len;
		return *this;
	}

	// 4. 删除中间或开头的子串
	// 将删除位置后面的内容前移
	size_t after_sub_pos = pos + otherStr._data_length;
	size_t tail_len = _data_length - after_sub_pos;

	// 移动后面的字符到删除位置
	memmove(_buffer + pos,
		_buffer + after_sub_pos,
		tail_len);

	_data_length = new_len;
	_buffer[_data_length] = '\0';

	return *this;
}

// 替换子串
bool String::replace(size_t pos, const String& subStr, const String& newStr)	
{
	if (pos >= _data_length)
	{
		std::cerr << "[ERROR] 位置越界" << std::endl;
		return false;
	}

	if (pos + subStr._data_length > _data_length)
	{
		std::cerr << "[ERROR] 替换子串超出原串范围" << std::endl;
		return false;
	}

	// 检查当前 pos 处是否匹配 subStr
	bool match = true;
	for (size_t i = 0; i < subStr._data_length; ++i)
	{
		if (_buffer[pos + i] != subStr._buffer[i])
		{
			match = false;
			break;
		}
	}
	if (!match)
	{
		std::cerr << "[ERROR] 指定位置不匹配替换子串" << std::endl;
		return false;
	}

	// 计算新长度
	size_t new_len = _data_length - subStr._data_length + newStr._data_length;

	// 判断是否需要扩容
	if (new_len >= _buffer_size)
	{
		renewBuffer(new_len + 1);
	}

	// 移动原串后半部分
	char* temp = new char[_buffer_size];
	size_t after_sub_pos = pos + subStr._data_length;
	size_t tail_len = _data_length - after_sub_pos;

	memcpy(temp, _buffer + after_sub_pos, tail_len);

	// 替换为新串
	memcpy(_buffer + pos, newStr._buffer, newStr._data_length);
	// 接上原串剩余部分
	memcpy(_buffer + pos + newStr._data_length, temp, tail_len);

	_data_length = new_len;
	_buffer[_data_length] = '\0';

	delete[] temp;
	return true;



}

// 查找子串
int String::find(const String& subStr) const
{
	return findSubstring(_buffer, _data_length, subStr._buffer, subStr._data_length);
}

// 查找子串
int String::find(const char* subStr) const
{
	if (!subStr) return -1;

	size_t sub_len = 0;
	while (subStr[sub_len] != '\0') sub_len++;

	return findSubstring(_buffer, _data_length, subStr, sub_len);

}



int String::findSubstring(const char* str, size_t str_len, const char* sub, size_t sub_len) const
{
	if (sub_len == 0 || sub_len > str_len) return -1;

	for (size_t i = 0; i <= str_len - sub_len; i++)
	{
		bool match = true;
		for (size_t j = 0; j < sub_len; j++)
		{
			if (str[i + j] != sub[j])
			{
				match = false;
				break;
			}
		}
		if (match) return static_cast<int>(i);
	}

	std::cerr << "[ERROR] 子串未找到！" << std::endl;
	return -1;
}

// buffer_size * 2
void String::renewBuffer(size_t requiredSize)
{
	size_t new_size = _buffer_size;

	while (new_size < requiredSize) new_size *= 2;

	char* newBuffer = new char[new_size];
	if (!newBuffer) {
		throw std::runtime_error("内存分配失败");
	}

	memcpy(newBuffer, _buffer, _data_length);
	newBuffer[_data_length] = '\0';

	delete[] _buffer;
	_buffer = newBuffer;
	_buffer_size = new_size;

}