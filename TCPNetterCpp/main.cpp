#include "tcp_client_wrapper.h"
#include <iostream>
#include <thread>

int main() {
    std::string host = "localhost";
    std::string port = "8188";
    std::string device_name = "DeviceX";
    std::string initial_message = "Initial Connection Message";

    // 创建 TCP 客户端包装类实例
    tcp_client_wrapper client(host, port, device_name, initial_message);

    // 创建一个线程用于持续读取服务器发送的信息
    //std::thread read_thread([&]() {
    //    while (true) {
    //        // 尝试读取服务器消息
    //        message_model model = client.get_model();
    //        if (!model.get_message().empty()) {
    //            //std::cout << "Received message: " << model.get_message() << std::endl;
    //        }

    //        // 每100毫秒检查一次
    //        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //    }
    //    });

    // 控制台输入处理
    while (true) {
        std::string input;
        std::cout << "Enter 'w' to write a message or 'q' to quit: ";
        std::cin >> input;

        if (input == "w") {
            std::cout << "Enter message to send: ";
            std::cin.ignore(); // 忽略之前的换行符
            std::getline(std::cin, input); // 获取输入的完整消息

            client.send_message(input); // 发送消息
        }
        else if (input == "q") {
            std::cout << "Exiting program..." << std::endl;
            break; // 退出循环，结束程序
        }
    }

    //// 程序退出时确保读取线程结束
    //if (read_thread.joinable()) {
    //    read_thread.join();
    //}

	return 0;
}
