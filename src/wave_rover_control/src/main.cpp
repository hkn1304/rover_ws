#include "rclcpp/rclcpp.hpp"
#include "wave_rover_control/joystick_driver.hpp"  // include your class header

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<joystick_to_json_pkg::JoystickDriver>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
