/**
 * @file const.h
 * @brief 常量定义文件
 * @details 定义了服务器通信相关的常量，包括消息长度、队列大小、消息ID等
 */

#pragma once

 // 消息最大长度（2KB）
#define MAX_LENGTH  1024*2

// 消息头总长度（字节）：消息ID(2字节) + 数据长度(2字节) = 4字节
#define HEAD_TOTAL_LEN 4

// 消息头中消息ID的长度（2字节）
#define HEAD_ID_LEN 2

// 消息头中数据长度的字段长度（2字节）
#define HEAD_DATA_LEN 2

// 接收队列最大容量
#define MAX_RECVQUE  10000

// 发送队列最大容量
#define MAX_SENDQUE 1000

/**
 * @brief 消息ID枚举
 * @details 定义系统中所有消息类型的ID
 */
enum MSG_IDS {
	MSG_HELLO_WORD = 1001  // Hello World消息ID
};
