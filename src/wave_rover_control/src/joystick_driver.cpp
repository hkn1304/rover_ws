#include "wave_rover_control/joystick_driver.hpp"

using std::placeholders::_1;

namespace joystick_to_json_pkg
{

JoystickDriver::JoystickDriver()
: Node("joystick_driver")
{
  joy_sub_ = this->create_subscription<sensor_msgs::msg::Joy>(
    "joy", 10, std::bind(&JoystickDriver::joy_callback, this, _1));
  json_pub_ = this->create_publisher<std_msgs::msg::String>("cmd_json_to_serial", 10);
}

void JoystickDriver::joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg)
{
  if (msg->axes.size() < 2) return;

  double left = msg->axes[1];  // Forward/backward on left stick
  double right = msg->axes[4]; // Forward/backward on right stick

  nlohmann::json cmd;
  cmd["T"] = 1;
  cmd["L"] = left;
  cmd["R"] = right;

  std_msgs::msg::String json_msg;
  json_msg.data = cmd.dump();
  json_pub_->publish(json_msg);
}

}
