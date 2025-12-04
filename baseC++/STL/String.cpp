#include "String.h"

String::String()
	:_buffer(nullptr), _buffer_size(0), _data_length(0)
{
	_buffer = new char[INIT_BUFF_SIZE];
	_buffer_size = INIT_BUFF_SIZE;
	_buffer[0] = '\0';
}

String::String(const String& otherStr)
	:_buffer(nullptr), _buffer_size(0), _data_length(otherStr._data_length)
{
	_buffer_size = otherStr._buffer_size;
	_buffer = new char[_buffer_size];
	memcpy(otherStr._buffer, _buffer, _data_length);
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

String::~String()
{
	delete[] _buffer;
	_buffer = nullptr;
}

// ×Ö·û´®Æ´½Ó
String String::operator+(const String& otherStr) const		
{

}

// É¾³ý×Ó´®
String String::operator-(const String& otherStr) const		
{

}

String& String::operator=(const String& otherStr)
{

}

// C×Ö·û´®¸³Öµ
String& String::operator=(const char* str)
{

}
// Ìæ»»×Ó´®
bool String::replace(size_t pos, const String& subStr, const String& newStr)	
{

}

// ²éÕÒ×Ó´®
int String::find(const String& subStr) const
{

}

// ²éÕÒ×Ó´®
int String::find(const char* subStr) const
{

}


// buffer_size * 2
void String::renewBuffer(size_t requiredSize)
{
	size_t new_size = _buffer_size;

	while (new_size < requiredSize) new_size *= 2;

	char* newBuffer = new char[new_size];
	if (!newBuffer) {
		throw std::runtime_error("ÄÚ´æ·ÖÅäÊ§°Ü");
	}

	memcpy(newBuffer, _buffer, _buffer_size);
	newBuffer[new_size] = '\0';

	delete[] _buffer;
	_buffer = newBuffer;
	_buffer_size = new_size;

}