#include "tcp_client_wrapper.h"

tcp_client_wrapper::tcp_client_wrapper(const std::string& host, const std::string& port, const std::string& device_name, const std::string& message)
    : client_(host, port, device_name, message), is_running_(true)
{
    write_thread_ = std::thread(&tcp_client_wrapper::write_thread_func, this);
    read_thread_ = std::thread(&tcp_client_wrapper::read_thread_func, this);
    heartbeat_thread_ = std::thread(&tcp_client_wrapper::heartbeat_thread_func, this);
}

tcp_client_wrapper::~tcp_client_wrapper()
{
    is_running_ = false;
    write_cv_.notify_all();
    read_cv_.notify_all();

    if (write_thread_.joinable()) write_thread_.join();
    if (read_thread_.joinable()) read_thread_.join();
    if (heartbeat_thread_.joinable()) heartbeat_thread_.join();
}

void tcp_client_wrapper::send_message(const std::string& message)
{
    std::lock_guard<std::mutex> lock(write_mutex_);
    write_queue_.push(message);
    write_cv_.notify_one();
}

message_model tcp_client_wrapper::get_model()
{
    return client_.get_model();
}

void tcp_client_wrapper::write_thread_func()
{
    while (is_running_) {
        std::unique_lock<std::mutex> lock(write_mutex_);
        write_cv_.wait(lock, [this] { return !write_queue_.empty() || !is_running_; });

        while (!write_queue_.empty()) {
            std::string message = write_queue_.front();
            write_queue_.pop();
            lock.unlock();

            client_.send_message(message);

            lock.lock();
        }
    }
}

void tcp_client_wrapper::read_thread_func()
{
    while (is_running_) {
        message_model model = client_.receive_message();
        std::lock_guard<std::mutex> lock(read_mutex_);
        read_queue_.push(model.get_message());
        read_cv_.notify_one();

        std::cout << "receive data, now model is: ------- \n" << model << "\n";
    }
}

void tcp_client_wrapper::heartbeat_thread_func()
{
    while (is_running_) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        client_.send_message_with_type("Heartbeats", "ping");
    }
}
