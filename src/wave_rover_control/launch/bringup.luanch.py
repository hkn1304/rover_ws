from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument
import os

def generate_launch_description():
    pkg_name = 'wave_rover_control'
    share_dir = os.path.join(
        os.getenv('COLCON_PREFIX_PATH').split(':')[0],
        'share', pkg_name
    )

    default_params = os.path.join(share_dir, 'config', 'params.yaml')
    ps5_config = os.path.join(share_dir, 'config', 'ps5_teleop.yaml')

    return LaunchDescription([
        DeclareLaunchArgument(
            'params_file',
            default_value=default_params,
            description='Path to ROS2 params YAML for json_to_serial_node'
        ),

        # Joystick driver
        Node(
            package='joy',
            executable='joy_node',
            name='joy_node',
            output='screen',
            parameters=[{'dev': '/dev/input/js0'}]
        ),

        # Teleop with PS5 config
        Node(
            package='teleop_twist_joy',
            executable='teleop_node',
            name='teleop_twist_joy',
            parameters=[ps5_config],
            remappings=[('/cmd_vel', '/cmd_vel_raw')]
        ),

        # Velocity smoother
        Node(
            package=pkg_name,
            executable='cmd_vel_smoother',
            name='cmd_vel_smoother',
            parameters=[{
                'accel_limit': 0.4,
                'decel_limit': 0.5,
                'control_rate': 20.0
            }],
            remappings=[('cmd_vel_raw', 'cmd_vel_raw'),
                        ('cmd_vel', 'cmd_vel')]
        ),

        # WaveRover serial bridge
        Node(
            package=pkg_name,
            executable='json_to_serial_node',
            name='json_to_serial_node',
            parameters=[LaunchConfiguration('params_file')],
            output='screen'
        )
    ])
