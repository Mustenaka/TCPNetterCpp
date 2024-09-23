#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

using boost::asio::ip::tcp;
using json = nlohmann::json;

class TCPClient {
public:
    TCPClient(const std::string& host, const std::string& port)
        : io_context_(), socket_(io_context_) {
        tcp::resolver resolver(io_context_);
        boost::asio::connect(socket_, resolver.resolve(host, port));
    }

    void sendMessage(const std::string& id, const std::string& messageType,
        const std::string& deviceName, const std::string& message,
        const std::string& command, const std::string& target) {

        // 创建 JSON 消息
        json j;
        j["Id"] = id;
        j["MessageType"] = messageType;
        j["DeviceName"] = deviceName;
        j["Message"] = message;
        j["Command"] = command;
        j["Target"] = target;

        // 转换为字符串并发送
        std::string request = j.dump() + "\n"; // 添加换行符以便服务器识别消息结束
        boost::asio::write(socket_, boost::asio::buffer(request));
    }

    std::string receiveMessage() {
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket_, buffer, "\n"); // 读取直到换行符
        std::istream input(&buffer);
        std::string response;
        std::getline(input, response);
        return response;
    }

    ~TCPClient() {
        socket_.close();
    }

private:
    boost::asio::io_context io_context_;
    tcp::socket socket_;
};

int main() {
    try {
        // 设置服务器IP和端口
        TCPClient client("127.0.0.1", "8188");

        // 发送JSON消息
        client.sendMessage("1234", "Message", "device1", "Hello, Server!", "", "");

        // 接收并打印服务器的响应
        std::string response = client.receiveMessage();
        std::cout << "Server response: " << response << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}