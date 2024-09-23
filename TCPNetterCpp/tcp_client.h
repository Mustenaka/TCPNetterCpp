#pragma once
#include <string>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <codecvt>

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

private:
	boost::asio::io_context io_context_;
	boost::asio::ip::tcp::socket socket_;
	message_model model_;
};
