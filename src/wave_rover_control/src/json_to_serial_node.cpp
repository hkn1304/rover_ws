#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <string>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <mutex>

class JsonToSerialNode : public rclcpp::Node
{
public:
  JsonToSerialNode()
  : Node("json_to_serial_node")
  {
    port_name_    = this->declare_parameter<std::string>("serial_port", "/dev/ttyUSB0");
    baudrate_     = this->declare_parameter<int>("baudrate", 115200);
    max_speed_    = this->declare_parameter<double>("max_speed", 0.5);
    timeout_sec_  = this->declare_parameter<double>("timeout_sec", 3.0);
    heartbeat_hz_ = this->declare_parameter<double>("heartbeat_hz", 5.0);

    if (!openSerial(port_name_, baudrate_)) {
      RCLCPP_FATAL(get_logger(), "Failed to open serial port: %s", port_name_.c_str());
      rclcpp::shutdown();
      return;
    }

    sub_cmd_vel_ = this->create_subscription<geometry_msgs::msg::Twist>(
      "/cmd_vel", 10,
      std::bind(&JsonToSerialNode::onCmdVel, this, std::placeholders::_1));

    timer_ = this->create_wall_timer(
      std::chrono::duration<double>(1.0 / heartbeat_hz_),
      std::bind(&JsonToSerialNode::onTimer, this));

    last_cmd_time_ = now();
    last_tx_time_ = now();
    RCLCPP_INFO(get_logger(), "WaveRover serial bridge started on %s @ %d baud",
                port_name_.c_str(), baudrate_);
  }

  ~JsonToSerialNode()
  {
    if (fd_ >= 0) {
      close(fd_);
    }
  }

private:
  bool openSerial(const std::string &device, int baudrate)
  {
    fd_ = ::open(device.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd_ < 0) {
      return false;
    }

    termios tio{};
    if (tcgetattr(fd_, &tio) != 0) {
      return false;
    }

    cfmakeraw(&tio);

    speed_t speed;
    switch (baudrate) {
      case 115200: speed = B115200; break;
      case 57600: speed = B57600; break;
      case 230400: speed = B230400; break;
      default: speed = B115200; break;
    }
    cfsetispeed(&tio, speed);
    cfsetospeed(&tio, speed);

    tio.c_cflag |= (CLOCAL | CREAD);
    tio.c_cflag &= ~CRTSCTS;
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;

    return (tcsetattr(fd_, TCSANOW, &tio) == 0);
  }

  void onCmdVel(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    double v = msg->linear.x;
    double w = msg->angular.z;

    double L = std::clamp(v - w, -max_speed_, max_speed_);
    double R = std::clamp(v + w, -max_speed_, max_speed_);

    {
      std::lock_guard<std::mutex> lock(mtx_);
      last_left_ = L;
      last_right_ = R;
      last_cmd_time_ = now();
    }

    sendSpeed(L, R);
  }

  void onTimer()
  {
    rclcpp::Time now_t = now();
    std::lock_guard<std::mutex> lock(mtx_);

    if ((now_t - last_cmd_time_).seconds() > timeout_sec_) {
      if (last_left_ != 0.0 || last_right_ != 0.0) {
        last_left_ = 0.0;
        last_right_ = 0.0;
        sendSpeed(0.0, 0.0);
        RCLCPP_WARN(get_logger(), "No command for %.1f s -> STOP",
                    (now_t - last_cmd_time_).seconds());
      }
      return;
    }

    if ((now_t - last_tx_time_).seconds() >= (1.0 / heartbeat_hz_ - 1e-3)) {
      sendSpeed(last_left_, last_right_);
    }
  }

  void sendSpeed(double L, double R)
  {
    char buf[128];
    int len = snprintf(buf, sizeof(buf),
                       "{\"T\":1,\"L\":%.3f,\"R\":%.3f}\n", L, R);
    if (write(fd_, buf, len) != len) {
      RCLCPP_ERROR(get_logger(), "Serial write failed");
    } else {
      last_tx_time_ = now();
    }
  }

  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_cmd_vel_;
  rclcpp::TimerBase::SharedPtr timer_;

  std::string port_name_;
  int baudrate_;
  int fd_{-1};

  double max_speed_;
  double timeout_sec_;
  double heartbeat_hz_;

  std::mutex mtx_;
  double last_left_{0.0}, last_right_{0.0};
  rclcpp::Time last_cmd_time_;
  rclcpp::Time last_tx_time_;
};

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<JsonToSerialNode>());
  rclcpp::shutdown();
  return 0;
}
