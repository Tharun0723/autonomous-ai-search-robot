
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():
    package_share = get_package_share_directory('robot_core')

    mission_file = os.path.join(
        package_share, 'config', 'mission.yaml'
    )

    locations_file = os.path.join(
        package_share, 'config', 'locations.yaml'
    )

    mission_controller = Node(
        package='robot_core',
        executable='mission_controller',
        name='mission_controller',
        output='screen',
        parameters=[{
            'mission_file': mission_file,
            'locations_file': locations_file,
        }]
    )

    return LaunchDescription([mission_controller])
