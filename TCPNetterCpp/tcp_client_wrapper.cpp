// ReSharper disable CppClangTidyClangDiagnosticInvalidUtf8
#include "tcp_client_wrapper.h"

/// <summary>
/// ���캯�����������������ӣ�wrapper���ö��߳�+����ģʽʵ����Ϣ�ķ��ͺͽ��յĶ��νӿ�
/// </summary>
/// <param name="host">������IP��ַ</param>
/// <param name="port">�������˿�</param>
/// <param name="device_name">�豸���ƣ���Ҫ����ȷ��Ψһ��</param>
/// <param name="message">��Ϣ����</param>
/// <param name="heartbeat_interval">�������ļ��ʱ��</param>
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
/// ����������������
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
/// ������Ϣ
/// </summary>
void tcp_client_wrapper::send_message(const std::string& message)
{
    std::lock_guard<std::mutex> lock(write_mutex_);
    write_queue_.push(message);
    write_cv_.notify_one();
}

/// <summary>
/// ��ȡ��ǰ��Ϣģ��
/// </summary>
message_model tcp_client_wrapper::get_model()
{
    return client_.get_model();
}

/// <summary>
/// д�̣߳��Զ����͡����з��͡����ִ��������ͷţ�����������Ҫ����ȥ��д��warp������ʱ����
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
/// ���̣߳��Զ���ȡ�����ж�ȡ�����ִ��������ͷţ�����������Ҫ����ȥ��д��warp������ʱ����
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
/// ������ - ÿ����һ������
/// </summary>
void tcp_client_wrapper::heartbeat_thread_func(const long long interval = 5)
{
    while (is_running_) {
        std::this_thread::sleep_for(std::chrono::seconds(interval));
        client_.send_message_with_type("Heartbeats", "ping");
    }
}
