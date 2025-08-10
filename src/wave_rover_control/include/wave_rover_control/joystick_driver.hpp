#ifndef JOYSTICK_TO_JSON_PKG__JOYSTICK_DRIVER_HPP_
#define JOYSTICK_TO_JSON_PKG__JOYSTICK_DRIVER_HPP_

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include <std_msgs/msg/string.hpp>
#include <nlohmann/json.hpp>

namespace joystick_to_json_pkg
{

class JoystickDriver : public rclcpp::Node
{
public:
  JoystickDriver();

private:
  void joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg);

  rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr json_pub_;
};

}  // namespace joystick_to_json_pkg

#endif  // JOYSTICK_TO_JSON_PKG__JOYSTICK_DRIVER_HPP_
