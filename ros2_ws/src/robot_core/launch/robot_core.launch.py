from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():

    config_file = os.path.join(
        get_package_share_directory('robot_core'),
        'config',
        'robot_params.yaml'
    )

    target_distance_arg = DeclareLaunchArgument(
        'target_distance',
        default_value='8.0',
        description='Target distance for the action client'
    )

    auto_start_goal_arg = DeclareLaunchArgument(
        'auto_start_goal',
        default_value='true',
        description='Automatically send the action goal when the client starts'
    )

    return LaunchDescription([

        target_distance_arg,
        auto_start_goal_arg,

        Node(
            package='robot_core',
            executable='robot_move_action_server',
            name='robot_move_action_server'
        ),

        Node(
            package='robot_core',
            executable='robot_move_action_client',
            name='robot_move_action_client',
            parameters=[
                config_file,
                {
                    'target_distance': LaunchConfiguration('target_distance'),
                    'auto_start_goal': LaunchConfiguration('auto_start_goal')
                }
            ]
        )

    ])
