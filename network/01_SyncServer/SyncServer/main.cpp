// 同步 服务端

#include <iostream>
#include <boost/asio.hpp>
#include <set>


const int max_length = 1024;	// 最大长度
typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;	// 别名
std::set<std::shared_ptr<std::thread>>  thread_set;	//线程集合

// 会话 - 一次连接的完整生命周期
void session(socket_ptr sock) {
	try {
		for (;;)
		{
			char data[max_length];
			memset(data, '\0', max_length);		// 初始化，每一位都设置为\0
			boost::system::error_code ec;
			size_t length = sock->read_some(boost::asio::buffer(data, max_length), ec);
			if (ec == boost::asio::error::eof) {
				std::cout << "connection closed by peer" << std::endl;
				break;
			}
			else if (ec) {
				throw boost::system::system_error(ec);
			}

			std::cout << "receive from " << sock->remote_endpoint().address().to_string() << std::endl;
			std::cout << "receive message is " << data << std::endl;
			//回传信息值
			boost::asio::write(*sock, boost::asio::buffer(data, length));

		}
	}
	catch (std::exception e) {
		std::cerr << "Exception in thread: " << e.what() << "\n" << std::endl;
	}
}

// 服务器主函数：创建服务器监听指定端口并接受客户端连接
void server(boost::asio::io_context& io_context, unsigned short port) {
    // 创建TCP接受器，绑定到指定端口，使用IPv4协议
    // tcp::v4() 表示使用IPv4协议
    // tcp::endpoint(tcp::v4(), port) 创建端点：所有网络接口的指定端口
    boost::asio::ip::tcp::acceptor a(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

    // 无限循环，持续接受客户端连接
    for (;;) {
        // 创建新的socket对象，用于与客户端通信
        // 使用智能指针管理socket生命周期
        socket_ptr socket(new boost::asio::ip::tcp::socket(io_context));

        // 阻塞等待，直到有客户端连接到来
        // accept操作会将新连接绑定到指定的socket对象
        a.accept(*socket);

        // 创建新线程处理客户端会话
        // std::make_shared 创建共享指针管理线程对象
        // session 是处理客户端通信的函数
        // socket 是传递给session函数的参数
        auto t = std::make_shared<std::thread>(session, socket);

        // 将线程指针加入到线程集合中，用于后续管理
        thread_set.insert(t);
    }
}

int main()
{
    try {
        boost::asio::io_context  ioc;
        server(ioc, 10086);
        for (auto& t : thread_set) {
            t->join();
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception " << e.what() << "\n";
    }
    return 0;
}