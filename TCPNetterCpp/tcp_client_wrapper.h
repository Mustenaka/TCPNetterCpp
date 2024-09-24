#pragma once
#include "tcp_client.h"

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <iostream>

class tcp_client_wrapper {
public:
    tcp_client_wrapper(const std::string& host, const std::string& port, const std::string& device_name, const std::string& message);
    ~tcp_client_wrapper();

    void send_message(const std::string& message);
    message_model get_model();

private:
    void write_thread_func();
    void read_thread_func();
    void heartbeat_thread_func();

    tcp_client client_;
    std::thread write_thread_;
    std::thread read_thread_;
    std::thread heartbeat_thread_;
    std::atomic<bool> is_running_;

    std::mutex write_mutex_;
    std::queue<std::string> write_queue_;
    std::condition_variable write_cv_;

    std::mutex read_mutex_;
    std::queue<std::string> read_queue_;
    std::condition_variable read_cv_;
};
