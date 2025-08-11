#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <chrono>

using namespace std::chrono_literals;

class CmdVelSmoother : public rclcpp::Node
{
public:
  CmdVelSmoother()
  : Node("cmd_vel_smoother")
  {
    accel_limit_ = this->declare_parameter<double>("accel_limit", 0.5); // m/s²
    decel_limit_ = this->declare_parameter<double>("decel_limit", 0.5); // m/s²
    control_rate_ = this->declare_parameter<double>("control_rate", 20.0); // Hz

    sub_ = create_subscription<geometry_msgs::msg::Twist>(
      "cmd_vel_raw", 10,
      std::bind(&CmdVelSmoother::onCmdVel, this, std::placeholders::_1));

    pub_ = create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);

    timer_ = create_wall_timer(
      std::chrono::duration<double>(1.0 / control_rate_),
      std::bind(&CmdVelSmoother::update, this));

    RCLCPP_INFO(get_logger(), "CmdVelSmoother running at %.1f Hz", control_rate_);
  }

private:
  void onCmdVel(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    target_linear_ = msg->linear.x;
    target_angular_ = msg->angular.z;
  }

  void update()
  {
    double dt = 1.0 / control_rate_;

    // Smooth linear velocity
    double diff_lin = target_linear_ - current_linear_;
    double max_delta_lin = (diff_lin > 0 ? accel_limit_ : decel_limit_) * dt;
    if (std::abs(diff_lin) > std::abs(max_delta_lin))
      current_linear_ += std::copysign(max_delta_lin, diff_lin);
    else
      current_linear_ = target_linear_;

    // Smooth angular velocity
    double diff_ang = target_angular_ - current_angular_;
    double max_delta_ang = (diff_ang > 0 ? accel_limit_ : decel_limit_) * dt;
    if (std::abs(diff_ang) > std::abs(max_delta_ang))
      current_angular_ += std::copysign(max_delta_ang, diff_ang);
    else
      current_angular_ = target_angular_;

    geometry_msgs::msg::Twist smoothed;
    smoothed.linear.x = current_linear_;
    smoothed.angular.z = current_angular_;
    pub_->publish(smoothed);
  }

  // ROS2
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  // Params
  double accel_limit_;
  double decel_limit_;
  double control_rate_;

  // State
  double target_linear_{0.0}, target_angular_{0.0};
  double current_linear_{0.0}, current_angular_{0.0};
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CmdVelSmoother>());
  rclcpp::shutdown();
  return 0;
}
