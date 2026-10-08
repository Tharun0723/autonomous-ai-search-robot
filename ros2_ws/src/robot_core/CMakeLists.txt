cmake_minimum_required(VERSION 3.16)

project(robot_core)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(yaml-cpp REQUIRED)

find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/GetRobotStatus.srv"
  "srv/NavigateToLocation.srv"
  "action/MoveRobot.action"
  DEPENDENCIES
  builtin_interfaces
)

add_executable(robot_status_node
  src/robot_status_node.cpp
)

ament_target_dependencies(robot_status_node
  rclcpp
  std_msgs
)

add_executable(robot_monitor_node
  src/robot_monitor_node.cpp
)

ament_target_dependencies(robot_monitor_node
  rclcpp
  std_msgs
)

add_executable(robot_status_server
  src/robot_status_server.cpp
)

ament_target_dependencies(robot_status_server
  rclcpp
  std_msgs
)

add_executable(robot_status_client
  src/robot_status_client.cpp
)

ament_target_dependencies(robot_status_client
  rclcpp
  std_msgs
)

rosidl_get_typesupport_target(
  cpp_typesupport_target
  ${PROJECT_NAME}
  "rosidl_typesupport_cpp"
)

target_link_libraries(
  robot_status_server
  "${cpp_typesupport_target}"
)

target_link_libraries(
  robot_status_client
  "${cpp_typesupport_target}"
)

add_executable(robot_move_action_server
  src/robot_move_action_server.cpp
)

ament_target_dependencies(robot_move_action_server
  rclcpp
  rclcpp_action
)

target_link_libraries(
  robot_move_action_server
  "${cpp_typesupport_target}"
)

add_executable(robot_move_action_client
  src/robot_move_action_client.cpp
)

ament_target_dependencies(robot_move_action_client
  rclcpp
  rclcpp_action
)

target_link_libraries(
  robot_move_action_client
  "${cpp_typesupport_target}"
)

add_executable(location_manager
  src/location_manager.cpp
)

ament_target_dependencies(location_manager
  rclcpp
  rclcpp_action
  nav2_msgs
)

target_link_libraries(
  location_manager
  yaml-cpp
)

target_link_libraries(
  location_manager
  "${cpp_typesupport_target}"
)

install(
  TARGETS
    robot_status_node
    robot_monitor_node
    robot_status_server
    robot_status_client
    robot_move_action_server
    robot_move_action_client
    location_manager
    DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY
    config
    launch
    description
    worlds
    maps
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
