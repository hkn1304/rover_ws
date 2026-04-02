cmake_minimum_required(VERSION 3.5)
project(wave_rover_control)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(include)

add_executable(joystick_driver_node src/main.cpp src/joystick_driver.cpp)
ament_target_dependencies(joystick_driver_node rclcpp sensor_msgs std_msgs nlohmann_json)

add_executable(json_to_serial_node src/json_to_serial_node.cpp)
ament_target_dependencies(json_to_serial_node rclcpp geometry_msgs std_msgs nlohmann_json)

add_executable(cmd_vel_smoother src/cmd_vel_smoother.cpp)
ament_target_dependencies(cmd_vel_smoother rclcpp geometry_msgs)
install(TARGETS
  cmd_vel_smoother
  joystick_driver_node
  json_to_serial_node
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY
  config
  launch
  DESTINATION share/${PROJECT_NAME}/)

install(DIRECTORY include/
  DESTINATION include)

ament_package()
