#include "tcp_client_wrapper.h"
#include <iostream>
#include <fcntl.h>

int main() {
	std::locale::global(std::locale(".utf8"));
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	const std::string host = "localhost";
	const std::string port = "8188";
	const std::string device_name = "DeviceX";
	const std::string initial_message = "Initial Connection Message";
	constexpr long long heartbeat_interval = 5;

	// 创建 TCP 客户端包装类实例
	tcp_client_wrapper client(host, port, device_name, initial_message, heartbeat_interval);

	// 控制台输入处理
	while (true) {
		std::string input;
		std::cout << "Enter 'w' to write a message or 'q' to quit or 'r' to read model: ";
		std::cin >> input;

		if (input == "w") {
			std::cout << "Enter message to send: ";
			std::cin.ignore(); // 忽略之前的换行符
			std::getline(std::cin, input); // 获取输入的完整消息

			client.send_message(input); // 发送消息
		}
		else if (input == "q") {
			std::cout << "Exiting program..." << '\n';
			break; // 退出循环，结束程序
		}
		else if (input == "r")
		{
			std::cout << "model is... \n" << client.get_model() << "\n";
		}
	}

	return 0;
}
