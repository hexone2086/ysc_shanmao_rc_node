#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "std_msgs/Bool.h"
#include "ysc_control/ysc_control_api.h"

#include <iomanip>
#include <sstream>

class YscControlNode
{
public:
  YscControlNode() : nh_("~")
  {
    // Initialize ControlClient
    control_client_ = new ControlClient();
    if (!control_client_->start()) {
      ROS_ERROR("Failed to start ControlClient");
      return;
    }
    ROS_INFO("ControlClient started successfully");

    // Subscribe to cmd_vel topic
    cmd_vel_sub_ = nh_.subscribe("cmd_vel", 10, &YscControlNode::cmd_vel_callback, this);

    // Subscribe to mode control topic
    mode_sub_ = nh_.subscribe("mode_control", 10, &YscControlNode::mode_control_callback, this);
  }

  ~YscControlNode()
  {
    delete control_client_;
  }

private:
  std::string get_current_time_str()
  {
    ros::Time now = ros::Time::now();
    std::time_t time = now.sec;
    std::tm* tm_info = std::localtime(&time);

    std::stringstream ss;
    ss << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S") << "." 
       << std::setfill('0') << std::setw(3) << static_cast<int>(now.nsec / 1000000);
    return ss.str();
  }

  void cmd_vel_callback(const geometry_msgs::Twist::ConstPtr& msg)
  {
    float x = msg->linear.x;
    float y = msg->linear.y;
    float yaw = msg->angular.z;

    if (control_client_->sendMotionData(x, y, yaw)) {
      ROS_INFO_STREAM(get_current_time_str() << " Motion data sent successfully");
    } else {
      ROS_ERROR_STREAM(get_current_time_str() << " Failed to send motion data");
    }
  }

  void mode_control_callback(const std_msgs::Bool::ConstPtr& msg)
  {
    bool mode = msg->data;
    if (mode) {
      if (control_client_->standUp(1)) {
        ROS_INFO_STREAM(get_current_time_str() << " Robot stand up successfully");
      } else {
        ROS_ERROR_STREAM(get_current_time_str() << " Failed to make robot stand up");
      }

      ros::Duration(0.5).sleep();

      if (control_client_->standUp(6)) {
        ROS_INFO_STREAM(get_current_time_str() << " Enter BaseMotion");
      } else {
        ROS_ERROR_STREAM(get_current_time_str() << " Failed to enter BaseMotion");
      }
    } else {
      if (control_client_->standUp(4)) {
        ROS_INFO_STREAM(get_current_time_str() << " Robot stand in mode 4 successfully");
      } else {
        ROS_ERROR_STREAM(get_current_time_str() << " Failed to make robot stand in mode 4");
      }
    }
  }

  ControlClient* control_client_;
  ros::NodeHandle nh_;
  ros::Subscriber cmd_vel_sub_;
  ros::Subscriber mode_sub_;
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "ysc_control_node");
  YscControlNode node;
  ros::spin();
  return 0;
}