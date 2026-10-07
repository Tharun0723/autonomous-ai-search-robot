from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.actions import RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, Command
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

import os


def generate_launch_description():

    package_share = get_package_share_directory('robot_core')

    # ---------------------------------------------------------
    # File paths
    # ---------------------------------------------------------

    controllers_file = os.path.join(
        package_share,
        'config',
        'controllers.yaml'
    )

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

    xacro_file = os.path.join(
        package_share,
        'description',
        'robot.urdf.xacro'
    )

    # ---------------------------------------------------------
    # Generate robot_description from Xacro
    # ---------------------------------------------------------

    robot_description = Command([
        'xacro ',
        xacro_file,
        ' controllers_file:=',
        controllers_file
    ])

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
        default_value='false',
        description='Automatically send the action goal when the client starts'
    )

    # ---------------------------------------------------------
    # Robot State Publisher
    # ---------------------------------------------------------

    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        parameters=[
            {
                'robot_description': robot_description
            }
        ],
        output='screen'
    )

    # ---------------------------------------------------------
    # Spawn robot into Gazebo
    # ---------------------------------------------------------

    spawn_robot = Node(
        package='ros_gz_sim',
        executable='create',
        arguments=[
            '-world', 'search_environment',
            '-string', robot_description,
            '-name', 'autonomous_search_robot',
            '-x', '0',
            '-y', '0',
            '-z', '0.2'
        ],
        output='screen'
    )

    # ---------------------------------------------------------
    # Controllers
    # ---------------------------------------------------------

    joint_state_broadcaster_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'joint_state_broadcaster',
            '--controller-manager',
            '/controller_manager',
            '--controller-manager-timeout',
            '60'
        ],
        output='screen'
    )

    diff_drive_controller_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'diff_drive_controller',
            '--controller-manager',
            '/controller_manager',
            '--controller-manager-timeout',
            '60'
        ],
        output='screen'
    )

    # ---------------------------------------------------------
    # Start controllers only after robot has spawned
    # ---------------------------------------------------------

    start_joint_state_broadcaster = RegisterEventHandler(
        OnProcessExit(
            target_action=spawn_robot,
            on_exit=[
                joint_state_broadcaster_spawner
            ]
        )
    )

    start_diff_drive_controller = RegisterEventHandler(
        OnProcessExit(
            target_action=joint_state_broadcaster_spawner,
            on_exit=[
                diff_drive_controller_spawner
            ]
        )
    )

    # ---------------------------------------------------------
    # Existing action server
    # ---------------------------------------------------------

    action_server = Node(
        package='robot_core',
        executable='robot_move_action_server',
        name='robot_move_action_server',
        output='screen'
    )

    # ---------------------------------------------------------
    # Existing action client
    # ---------------------------------------------------------

    action_client = Node(
        package='robot_core',
        executable='robot_move_action_client',
        name='robot_move_action_client',
        parameters=[
            config_file,
            {
                'target_distance': LaunchConfiguration('target_distance'),
                'auto_start_goal': LaunchConfiguration('auto_start_goal')
            }
        ],
        output='screen'
    )

    # ---------------------------------------------------------
    # Launch everything
    # ---------------------------------------------------------

    return LaunchDescription([
        target_distance_arg,
        auto_start_goal_arg,

        gazebo_launch,

        robot_state_publisher,

        spawn_robot,

        start_joint_state_broadcaster,
        start_diff_drive_controller,

        action_server,
        action_client
    ])
