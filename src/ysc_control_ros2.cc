#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/bool.hpp"
#include "ysc_control/ysc_control_api.h"

#include <iomanip>
#include <sstream>

class YscControlNode : public rclcpp::Node
{
public:
  YscControlNode()
  : Node("ysc_control_node")
  {
    // 初始化 ControlClient
    control_client_ = std::make_unique<ControlClient>();
    if (!control_client_->start()) {
      RCLCPP_ERROR(this->get_logger(), "Failed to start ControlClient");
      return;
    }
    RCLCPP_INFO(this->get_logger(), "ControlClient started successfully");

    // 订阅 cmd_vel 话题
    subscription_ = this->create_subscription<geometry_msgs::msg::Twist>(
      "cmd_vel", 10, std::bind(&YscControlNode::cmd_vel_callback, this, std::placeholders::_1));

    // 订阅模式控制话题
    mode_subscription_ = this->create_subscription<std_msgs::msg::Bool>(
      "mode_control", 10, std::bind(&YscControlNode::mode_control_callback, this, std::placeholders::_1));
  }

private:
  std::string get_current_time_str()
  {
    auto now = this->get_clock()->now();
    auto time_point = now.seconds();
    std::time_t time = static_cast<std::time_t>(time_point);
    std::tm* tm_info = std::localtime(&time);

    std::stringstream ss;
    ss << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S") << "." 
       << std::setfill('0') << std::setw(3) << static_cast<int>((time_point - static_cast<double>(time)) * 1000);
    return ss.str();
  }

  void cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    float x = msg->linear.x;
    float y = msg->linear.y;
    float yaw = msg->angular.z;

    if (control_client_->sendMotionData(x, y, yaw)) {
      RCLCPP_INFO_STREAM(this->get_logger(), get_current_time_str() << " Motion data sent successfully");
    } else {
      RCLCPP_ERROR_STREAM(this->get_logger(), get_current_time_str() << " Failed to send motion data");
    }
  }

  void mode_control_callback(const std_msgs::msg::Bool::SharedPtr msg)
  {
    bool mode = msg->data;
    if (mode) {
      if (control_client_->standUp(1)) {
        RCLCPP_INFO_STREAM(this->get_logger(), get_current_time_str() << " Robot stand up successfully");
      } else {
        RCLCPP_ERROR_STREAM(this->get_logger(), get_current_time_str() << " Failed to make robot stand up");
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      if (control_client_->standUp(6)) {
        RCLCPP_INFO_STREAM(this->get_logger(), get_current_time_str() << " Enter BaseMotion");
      } else {
        RCLCPP_ERROR_STREAM(this->get_logger(), get_current_time_str() << " Failed to enter BaseMotion");
      }
    } else {
      if (control_client_->standUp(4)) {
        RCLCPP_INFO_STREAM(this->get_logger(), get_current_time_str() << " Robot stand in mode 4 successfully");
      } else {
        RCLCPP_ERROR_STREAM(this->get_logger(), get_current_time_str() << " Failed to make robot stand in mode 4");
      }
    }
  }

  std::unique_ptr<ControlClient> control_client_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscription_;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr mode_subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<YscControlNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
