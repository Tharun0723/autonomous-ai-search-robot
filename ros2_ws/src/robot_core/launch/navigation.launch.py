from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

import os


def generate_launch_description():

    package_share = get_package_share_directory('robot_core')

    nav2_params_file = os.path.join(
        package_share,
        'config',
        'nav2_params.yaml'
    )

    nav2_nodes = [

        Node(
            package='nav2_planner',
            executable='planner_server',
            name='planner_server',
            output='screen',
            parameters=[nav2_params_file]
        ),

        Node(
            package='nav2_controller',
            executable='controller_server',
            name='controller_server',
            output='screen',
            parameters=[nav2_params_file],
            remappings=[
                ('cmd_vel', '/diff_drive_controller/cmd_vel')
            ]
        ),

        Node(
            package='nav2_behaviors',
            executable='behavior_server',
            name='behavior_server',
            output='screen',
            parameters=[nav2_params_file],
            remappings=[
                ('cmd_vel', '/diff_drive_controller/cmd_vel')
            ]
        ),

        Node(
            package='nav2_bt_navigator',
            executable='bt_navigator',
            name='bt_navigator',
            output='screen',
            parameters=[nav2_params_file]
        ),

        Node(
            package='nav2_waypoint_follower',
            executable='waypoint_follower',
            name='waypoint_follower',
            output='screen',
            parameters=[nav2_params_file]
        ),

        Node(
            package='nav2_lifecycle_manager',
            executable='lifecycle_manager',
            name='lifecycle_manager_navigation',
            output='screen',
            parameters=[
                {
                    'use_sim_time': True,
                    'autostart': True,
                    'node_names': [
                        'planner_server',
                        'controller_server',
                        'behavior_server',
                        'bt_navigator',
                        'waypoint_follower'
                    ]
                }
            ]
        )
    ]

    return LaunchDescription(nav2_nodes)
