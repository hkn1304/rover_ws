cmake_minimum_required(VERSION 3.5)
project(wave_rover_logitech_f710_teleop)

find_package(ament_cmake REQUIRED)

install(DIRECTORY
  config
  launch
  DESTINATION share/${PROJECT_NAME}/)

ament_package()
