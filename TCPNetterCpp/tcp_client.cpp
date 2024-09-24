#include "tcp_client.h"

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
	this->port_ = port;
	this->host_ = host;
	this->connected_ = false;

	reconnect();
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
	// 如果是断开链接的状态下，则重新链接
	if (!is_connected())
	{
		reconnect();
	}

	// 如果是Message类型的信息，那么就保存到model中
	if (message_type == "Message")
	{
		model_.set_message(message);
	}

	nlohmann::json j;

	try 
	{
		// 检查是否是有效的 UTF-8 编码
		if (!tcp_client::is_valid_utf8(message)) {
			std::cerr << "Error: Invalid UTF-8 message." << std::endl;
			return; // 不发送无效的消息
		}

		j["Id"] = model_.get_id();
		j["MessageType"] = message_type;
		j["DeviceName"] = model_.get_device_name();
		j["Message"] = message;
		j["Command"] = "";
		j["Target"] = "";

		std::string request = j.dump() + "\n";
		boost::asio::write(socket_, boost::asio::buffer(request));
	}
	catch (const nlohmann::json::exception& e) 
	{
		std::cerr << "JSON error: " << e.what() << '\n';
	}
	catch (const boost::system::system_error& e) 
	{
		std::cerr << "Send failed: " << e.what() << '\n';

		// 检查特定的错误类型
		if (e.code() == boost::asio::error::host_not_found ||
			e.code() == boost::asio::error::connection_refused ||
			e.code() == boost::asio::error::network_unreachable) {
			// 这些错误可以尝试重连
			connected_ = false;
			reconnect();
		}
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

	try
	{
		// 检查是否是有效的 UTF-8 编码
		if (!tcp_client::is_valid_utf8(response)) {
			std::cerr << "Error: Invalid UTF-8 message." << std::endl;
			return model_; // 不发送无效的消息
		}

		nlohmann::json j = nlohmann::json::parse(response);

		// 只有是Message类型的信息才会进行保存
		if (j["MessageType"] == "Message")
		{
			model_.set_id(j["Id"]);
			model_.set_message(j["Message"]);
			model_.set_command(j["Command"]);
			model_.set_target(j["Target"]);

			this->send_message_with_type("Callback", j["Message"]);
		}

		std::cout << "Received message: " << response << '\n';
	}
	catch (const nlohmann::json::exception& e)
	{
		std::cerr << "JSON error: " << e.what() << '\n';
	}
	catch (const boost::system::system_error& e) 
	{
		std::cerr << "Receive failed: " << e.what() << '\n';

		// 检查特定的错误类型
		if (e.code() == boost::asio::error::host_not_found ||
			e.code() == boost::asio::error::connection_refused ||
			e.code() == boost::asio::error::network_unreachable) {
			// 这些错误可以尝试重连
			connected_ = false;
			reconnect();
		}
	}

	return model_;
}

/// <summary>
/// 判断当前链接状态
/// </summary>
bool tcp_client::is_connected() const
{
	return this->connected_;
}

/// <summary>
/// 重新链接服务器
/// </summary>
void tcp_client::reconnect()
{
	boost::asio::ip::tcp::resolver resolver(io_context_);
	const boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host_, port_);

	while (true) {
		try {
			boost::asio::connect(socket_, endpoints);
			connected_ = true;
			std::cout << "Connected to server: " << host_ << ":" << port_ << '\n';
			break;  // 连接成功，退出循环
		}
		catch (const boost::system::system_error& e) {
			connected_ = false;
			std::cerr << "Connection failed: " << e.what() << '\n';
			std::cout << "Retrying connection in 3 seconds..." << '\n';
			std::this_thread::sleep_for(std::chrono::seconds(3));  // 等待3秒后重试
		}
	}
}

/// <summary>
/// 检查数据是否是utf-8
/// </summary>
bool tcp_client::is_valid_utf8(const std::string& str)
{
	// 使用 std::wstring_convert 进行编码检查
	try {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		auto receive = converter.from_bytes(str);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

/// <summary>
/// string字符串转换wstring字符串（utf-8）支持
/// </summary>
std::wstring tcp_client::string_convert_utf8(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(str);
}
