#include "tcp_client.h"

#include <iostream>

/// <summary>
/// 构造函数，创建tcp客户端
/// </summary>
/// <param name="host">目标地址</param>
/// <param name="port">端口</param>
/// <param name="device_name">设备名称（自行保证唯一）</param>
/// <param name="message">消息信息</param>
tcp_client::tcp_client(const std::string& host, const std::string& port,
	const std::string& device_name, const std::string& message) : io_context_(), socket_(io_context_), model_(
		"",
		"message",
		device_name,
		message,
		"",
		"")
{
	boost::asio::ip::tcp::resolver resolver(io_context_);
	boost::asio::connect(socket_, resolver.resolve(host, port));
}

/// <summary>
/// 析构函数
/// </summary>
tcp_client::~tcp_client()
{
	socket_.close();
}

/// <summary>
/// 获取model模型
/// </summary>
/// <returns>模型信息</returns>
message_model tcp_client::get_model()
{
	return model_;
}

/// <summary>
/// 发送信息 | 只发送Message类型的信息
/// </summary>
void tcp_client::send_message(const std::string& message)
{
	this->send_message_with_type("Message", message);
}

/// <summary>
/// 发送消息 | 带有Type类型的信息
/// </summary>
/// <param name="message_type">消息类型</param>
/// <param name="message">消息</param>
void tcp_client::send_message_with_type(const std::string& message_type, const std::string& message)
{
	// 如果是Message类型的信息，那么就保存到model中
	if (message_type == "Message")
	{
		model_.set_message(message);
	}

	nlohmann::json j;

	try {
		j["Id"] = model_.get_id();
		j["MessageType"] = message_type;
		j["DeviceName"] = model_.get_device_name();
		j["Message"] = message;
		j["Command"] = "";
		j["Target"] = "";

		std::string request = j.dump() + "\n";
		boost::asio::write(socket_, boost::asio::buffer(request));
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << "JSON error: " << e.what() << std::endl;
	}
}

/// <summary>
/// 接受信息, 会存在解析信息修改自身message信息
/// </summary>
/// <returns></returns>
message_model tcp_client::receive_message()
{
	boost::asio::streambuf buffer;
	boost::asio::read_until(socket_, buffer, "\n");
	std::istream input(&buffer);
	std::string response;
	std::getline(input, response);

	nlohmann::json j = nlohmann::json::parse(response);

	// 只有是Message类型的信息才会进行保存
	if (j["Message"] == "Message")
	{
		model_.set_id(j["Id"]);
		model_.set_message_type(j["MessageType"]);
		model_.set_device_name(j["DeviceName"]);
		model_.set_message(j["Message"]);
		model_.set_command(j["Command"]);
		model_.set_target(j["Target"]);

		this->send_message_with_type("Callback", j["Message"]);
	}

	std::cout << "Received message: " << response << std::endl;

	return model_;
}

std::string wstring_to_string(const std::wstring& wstr) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}