from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

import os


def generate_launch_description():

    package_share = get_package_share_directory('robot_core')

    locations_file = os.path.join(
        package_share,
        'config',
        'locations.yaml'
    )

    location_manager = Node(
        package='robot_core',
        executable='location_manager',
        name='location_manager',
        output='screen',
        parameters=[
            {
                'locations_file': locations_file
            }
        ]
    )

    return LaunchDescription([
        location_manager
    ])
