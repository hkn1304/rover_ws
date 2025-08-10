from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='wave_rover_control',
            executable='joystick_driver_node',
            name='joystick_driver_node'
        ),
        Node(
            package='wave_rover_control',
            executable='json_to_serial_node',
            name='json_to_serial_node'
        )
    ])
