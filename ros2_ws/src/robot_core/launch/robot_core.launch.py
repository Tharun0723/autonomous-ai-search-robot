from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():

    package_share = get_package_share_directory('robot_core')

    # ---------------------------------------------------------
    # File paths
    # ---------------------------------------------------------

    world_file = os.path.join(
        package_share,
        'worlds',
        'search_environment.sdf'
    )

    bridge_config_file = os.path.join(
        package_share,
        'config',
        'gazebo_bridge.yaml'
    )

    config_file = os.path.join(
        package_share,
        'config',
        'robot_params.yaml'
    )

    urdf_file = os.path.join(
        package_share,
        'description',
        'robot.urdf'
    )

    # ---------------------------------------------------------
    # Gazebo
    # ---------------------------------------------------------

    gazebo_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('ros_gz_sim'),
                'launch',
                'ros_gz_sim.launch.py'
            )
        ),
        launch_arguments={
            'bridge_name': 'ros_gz_bridge',
            'config_file': bridge_config_file,
            'world_sdf_file': world_file
        }.items()
    )

    # ---------------------------------------------------------
    # Launch arguments
    # ---------------------------------------------------------

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

    # ---------------------------------------------------------
    # Robot description
    # ---------------------------------------------------------

    with open(urdf_file, 'r') as file:
        robot_description = file.read()

    # ---------------------------------------------------------
    # Launch all nodes
    # ---------------------------------------------------------

    return LaunchDescription([

        # Launch arguments
        target_distance_arg,
        auto_start_goal_arg,

        # Gazebo simulation
        gazebo_launch,

        # Robot State Publisher
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            parameters=[
                {
                    'robot_description': robot_description
                }
            ]
        ),

        # Robot movement action server
        Node(
            package='robot_core',
            executable='robot_move_action_server',
            name='robot_move_action_server'
        ),

        # Robot movement action client
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
