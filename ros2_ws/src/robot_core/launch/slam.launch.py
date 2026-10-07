from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():

    robot_core_share = get_package_share_directory('robot_core')
    slam_toolbox_share = get_package_share_directory('slam_toolbox')

    slam_params_file = os.path.join(
        robot_core_share,
        'config',
        'slam_params.yaml'
    )

    return LaunchDescription([

        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(
                    slam_toolbox_share,
                    'launch',
                    'online_async_launch.py'
                )
            ),
            launch_arguments={
                'slam_params_file': slam_params_file
            }.items()
        )

    ])
