#include "tcp_client_wrapper.h"

tcp_client_wrapper::tcp_client_wrapper(const std::string& host, const std::string& port,
	const std::string& device_name, const std::string& message)
	: client_(host, port, device_name, message), is_running_(true) {

	write_thread_ = std::thread(&tcp_client_wrapper::write_thread_func, this);
	read_thread_ = std::thread(&tcp_client_wrapper::read_thread_func, this);
	heartbeat_thread_ = std::thread(&tcp_client_wrapper::heartbeat_thread_func, this);
}

tcp_client_wrapper::~tcp_client_wrapper() {
	is_running_ = false;
	if (write_thread_.joinable()) {
		write_thread_.join();
	}
	if (read_thread_.joinable()) {
		read_thread_.join();
	}
	if (heartbeat_thread_.joinable()) {
		heartbeat_thread_.join();
	}
}

void tcp_client_wrapper::send_message(const std::string& message) {
	std::lock_guard<std::mutex> lock(mutex_);
	client_.send_message(message);
}

message_model tcp_client_wrapper::get_model() {
	return client_.get_model();
}

void tcp_client_wrapper::write_thread_func() {
	while (is_running_) {
		// 这里可以实现你的写入逻辑，例如从队列中读取消息并发送
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void tcp_client_wrapper::read_thread_func() {
	while (is_running_) {
		message_model model = client_.receive_message();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void tcp_client_wrapper::heartbeat_thread_func() {
	while (is_running_) {
		std::this_thread::sleep_for(std::chrono::seconds(30));
		std::lock_guard<std::mutex> lock(mutex_);
		client_.send_message_with_type("Heartbeats", "ping");
	}
}
