from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
from launch.substitutions import LaunchConfiguration
import os

def generate_launch_description():
    pkg_name = 'wave_rover_control'
    share_dir = get_package_share_directory(pkg_name)

    default_params = os.path.join(share_dir, 'config', 'params.yaml')
    return LaunchDescription([
        DeclareLaunchArgument(
            'params_file',
            default_value=default_params,
            description='Path to ROS2 params YAML for json_to_serial_node'
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
            remappings=[('cmd_vel_raw', '/cmd_vel_raw'),
                        ('cmd_vel', '/cmd_vel')],
            output='screen'
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
