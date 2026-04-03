from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():
    teleop_share_dir = get_package_share_directory('wave_rover_ps5_teleop')
    control_share_dir = get_package_share_directory('wave_rover_control')

    joy_dev = LaunchConfiguration('joy_dev')
    params_file = LaunchConfiguration('params_file')
    ps5_config = os.path.join(teleop_share_dir, 'config', 'ps5_teleop.yaml')
    default_params = os.path.join(control_share_dir, 'config', 'params.yaml')

    return LaunchDescription([
        DeclareLaunchArgument(
            'joy_dev',
            default_value='/dev/input/js0',
            description='Joystick device for the PS5 controller'
        ),
        DeclareLaunchArgument(
            'params_file',
            default_value=default_params,
            description='Path to Wave Rover serial bridge parameters'
        ),

        Node(
            package='joy',
            executable='joy_node',
            name='joy_node',
            output='screen',
            parameters=[{'dev': joy_dev}]
        ),

        Node(
            package='teleop_twist_joy',
            executable='teleop_node',
            name='teleop_twist_joy',
            output='screen',
            parameters=[ps5_config],
            remappings=[('/cmd_vel', '/cmd_vel_raw')]
        ),

        Node(
            package='wave_rover_control',
            executable='cmd_vel_smoother',
            name='cmd_vel_smoother',
            output='screen',
            parameters=[{
                'accel_limit': 0.4,
                'decel_limit': 0.5,
                'control_rate': 20.0
            }],
            remappings=[('cmd_vel_raw', '/cmd_vel_raw'),
                        ('cmd_vel', '/cmd_vel')]
        ),

        Node(
            package='wave_rover_control',
            executable='json_to_serial_node',
            name='json_to_serial_node',
            output='screen',
            parameters=[params_file]
        )
    ])
