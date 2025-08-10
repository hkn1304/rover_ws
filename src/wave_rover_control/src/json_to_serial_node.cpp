#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

class JsonToSerialNode : public rclcpp::Node
{
public:
  JsonToSerialNode() : Node("json_to_serial_node")
  {
    serial_port_.open("/dev/ttyUSB0", std::ios::out | std::ios::binary);
    if (!serial_port_.is_open())
    {
      RCLCPP_ERROR(this->get_logger(), "Failed to open /dev/ttyUSB0");
      rclcpp::shutdown();
      return;
    }

    sub_ = this->create_subscription<std_msgs::msg::String>(
      "cmd_json_to_serial", 10,
      [this](const std_msgs::msg::String::SharedPtr msg)
      {
        serial_port_ << msg->data << "\n";
        serial_port_.flush();
      });
  }

  ~JsonToSerialNode()
  {
    if (serial_port_.is_open())
    {
      serial_port_.close();
    }
  }

private:
  std::ofstream serial_port_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<JsonToSerialNode>());
  rclcpp::shutdown();
  return 0;
}
