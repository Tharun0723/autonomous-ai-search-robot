from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():

    config_file = os.path.join(
        get_package_share_directory('robot_core'),
        'config',
        'robot_params.yaml'
    )

    return LaunchDescription([

        Node(
            package='robot_core',
            executable='robot_move_action_server',
            name='robot_move_action_server'
        ),

        Node(
            package='robot_core',
            executable='robot_move_action_client',
            name='robot_move_action_client',
            parameters=[config_file]
        )

    ])
