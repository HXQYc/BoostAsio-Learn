// 同步 客户端

#include <iostream>
#include <boost/asio.hpp>

const int MAX_LENGTH = 1024;
int main()
{
	try
	{
		// 创建 上下文 服务
		boost::asio::io_context ioc;

		// 构造 endpoint
		boost::asio::ip::tcp::endpoint remote_ep(
			boost::asio::ip::make_address("127.0.0.1"),
			10086
		);

		// 创建 socket
		boost::asio::ip::tcp::socket sock(ioc);

		// 错误码，初值为无法找的主机
		boost::system::error_code ec = boost::asio::error::host_not_found;

		// 连接
		sock.connect(remote_ep, ec);

		// 判断
		if (ec) {
			std::cout << "connect failed, code is " << ec.value() << " error msg is " << ec.message();
			std::cout << std::endl;
			return 0;
		}

		// 输入传输的数据
		std::cout << "Enter message: ";
		char request[MAX_LENGTH];
		std::cin.getline(request, MAX_LENGTH);
		// 长度
		size_t request_length = strlen(request);
		// 发送数据
		boost::asio::write(sock, boost::asio::buffer(request, request_length));

		// 接收数据
		char reply[MAX_LENGTH];
		size_t reply_length = boost::asio::read(sock,
			boost::asio::buffer(reply, request_length));
		std::cout << "Reply is: ";

		// 把收到的数据回传
		std::cout.write(reply, reply_length);
		std::cout << "\n";
		getchar();

	}
	catch(std::exception e){
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
