#pragma once
#include <string>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <codecvt>
#include <iostream>
#include <locale>

#include "message_model.h"

class tcp_client
{
public:
	tcp_client(const std::string& host, const std::string& port, const std::string& device_name, const std::string& message);
	~tcp_client();

	message_model get_model();
	void send_message(const std::string& message);
	void send_message_with_type(const std::string& message_type, const std::string& message);
	message_model receive_message();

	bool is_connected() const;
	void reconnect();

private:
	boost::asio::io_context io_context_;
	boost::asio::ip::tcp::socket socket_;
	message_model model_;
	bool connected_;
	std::string host_;
	std::string port_;

	static bool is_valid_utf8(const std::string& str);
	static std::wstring string_convert_utf8(const std::string& str);
};
