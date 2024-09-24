// ReSharper disable CppClangTidyClangDiagnosticInvalidUtf8
#include "tcp_client_wrapper.h"

/// <summary>
/// 构造函数，创建服务器链接，wrapper利用多线程+队列模式实现消息的发送和接收的二次接口
/// </summary>
/// <param name="host">服务器IP地址</param>
/// <param name="port">服务器端口</param>
/// <param name="device_name">设备名称（需要自行确认唯一）</param>
/// <param name="message">消息内容</param>
/// <param name="heartbeat_interval">心跳包的间隔时间</param>
tcp_client_wrapper::tcp_client_wrapper(
    const std::string& host, const std::string& port, 
    const std::string& device_name, const std::string& message,
    const long long heartbeat_interval)
    : client_(host, port, device_name, message), is_running_(true)
{
    write_thread_ = std::thread(&tcp_client_wrapper::write_thread_func, this);
    read_thread_ = std::thread(&tcp_client_wrapper::read_thread_func, this);
    heartbeat_thread_ = std::thread(&tcp_client_wrapper::heartbeat_thread_func, this, heartbeat_interval);

	this->heartbeat_interval_ = heartbeat_interval;
}

/// <summary>
/// 析构函数，销毁用
/// </summary>
tcp_client_wrapper::~tcp_client_wrapper()
{
    is_running_ = false;
    write_cv_.notify_all();
    read_cv_.notify_all();

    if (write_thread_.joinable()) write_thread_.join();
    if (read_thread_.joinable()) read_thread_.join();
    if (heartbeat_thread_.joinable()) heartbeat_thread_.join();
}

/// <summary>
/// 发送消息
/// </summary>
void tcp_client_wrapper::send_message(const std::string& message)
{
    std::lock_guard<std::mutex> lock(write_mutex_);
    write_queue_.push(message);
    write_cv_.notify_one();
}

/// <summary>
/// 获取当前消息模型
/// </summary>
message_model tcp_client_wrapper::get_model()
{
    return client_.get_model();
}

/// <summary>
/// 写线程，自动发送、队列发送、容灾处理，错误释放，重连，不需要主动去复写，warp层启动时启动
/// </summary>
void tcp_client_wrapper::write_thread_func()
{
    try
    {
        while (is_running_)
        {
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
	catch(boost::exception& e)
	{
		std::cout << "boost exception(failed to send message to service): " << '\n';
        client_.reconnect();
	}
	catch (std::exception& e)
	{
		std::cout << "std exception: " << e.what() << '\n';
	}
	catch (...)
	{
		std::cout << "unknown exception" << '\n';
	}
}

/// <summary>
/// 读线程，自动读取、队列读取、容灾处理，错误释放，重连，不需要主动去复写，warp层启动时启动
/// </summary>
void tcp_client_wrapper::read_thread_func()
{
    try
    {
        while (is_running_) {
            message_model model = client_.receive_message();
            std::lock_guard<std::mutex> lock(read_mutex_);
            read_queue_.push(model.get_message());
            read_cv_.notify_one();

            std::cout << "receive data, now model is: ------- \n" << model << "\n";
        }
	}
	catch (boost::exception& e)
	{
		std::cout << "boost exception: (failed to receive message from service)" << '\n';
        client_.reconnect();
	}
	catch (std::exception& e)
	{
		std::cout << "std exception: " << e.what() << '\n';
	}
	catch (...)
	{
		std::cout << "unknown exception" << '\n';
	}

}

/// <summary>
/// 心跳包 - 每五秒一个心跳
/// </summary>
void tcp_client_wrapper::heartbeat_thread_func(const long long interval = 5)
{
    while (is_running_) {
        std::this_thread::sleep_for(std::chrono::seconds(interval));
        client_.send_message_with_type("Heartbeats", "ping");
    }
}
