#pragma once
#include<string>

class message_model
{
public:
    message_model(std::string id, std::string message_type, std::string device_name,
                 std::string message, std::string command, std::string target)
        : id_(std::move(id)), message_type_(std::move(message_type)), device_name_(std::move(device_name)),
        message_(std::move(message)), command_(std::move(command)), target_(std::move(target))
    {
}

    std::string get_id() const { return id_; }
    std::string get_message_type() const { return message_type_; }
    std::string get_device_name() const { return device_name_; }
    std::string get_message() const { return message_; }
    std::string get_command() const { return command_; }
    std::string get_target() const { return target_; }

    void set_id(const std::string& id) { id_ = id; }
    void set_message_type(const std::string& message_type) { message_type_ = message_type; }
    void set_device_name(const std::string& device_name) { device_name_ = device_name; }
    void set_message(const std::string& message) { message_ = message; }
    void set_command(const std::string& command) { command_ = command; }
    void set_target(const std::string& target) { target_ = target; }

    friend std::ostream& operator<<(std::ostream& os, const message_model& model) {
        os << "ID: " << model.id_ << "\n"
            << "MessageType: " << model.message_type_ << "\n"
            << "DeviceName: " << model.device_name_ << "\n"
            << "Message: " << model.message_ << "\n"
            << "Command: " << model.command_ << "\n"
            << "Target: " << model.target_;
        return os;
    }

private:
	std::string id_;            // ID   // NOLINT(clang-diagnostic-invalid-utf8)
	std::string message_type_;  // ��Ϣ���� // NOLINT(clang-diagnostic-invalid-utf8)
	std::string device_name_;   // �豸���� // NOLINT(clang-diagnostic-invalid-utf8)
	std::string message_;       // ��Ϣ���� // NOLINT(clang-diagnostic-invalid-utf8)
	std::string command_;       // ����   // NOLINT(clang-diagnostic-invalid-utf8)
	std::string target_;        // Ŀ��   // NOLINT(clang-diagnostic-invalid-utf8)
};