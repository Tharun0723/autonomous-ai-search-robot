cmake_minimum_required(VERSION 3.8)
project(robot_core)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ROS 2 dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# Generate custom ROS 2 interfaces
rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/GetRobotStatus.srv"
  "action/MoveRobot.action"
)

# Testing
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# ---------------------------------------------------------
# Robot Status Publisher Node
# ---------------------------------------------------------

add_executable(robot_status_node src/robot_status_node.cpp)

ament_target_dependencies(robot_status_node
  rclcpp
  std_msgs
)

# ---------------------------------------------------------
# Robot Monitor Subscriber Node
# ---------------------------------------------------------

add_executable(robot_monitor_node src/robot_monitor_node.cpp)

ament_target_dependencies(robot_monitor_node
  rclcpp
  std_msgs
)

# ---------------------------------------------------------
# Robot Status Service Server
# ---------------------------------------------------------

add_executable(robot_status_server src/robot_status_server.cpp)

ament_target_dependencies(robot_status_server
  rclcpp
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

# ---------------------------------------------------------
# Robot Status Service Client
# ---------------------------------------------------------

add_executable(robot_status_client src/robot_status_client.cpp)

ament_target_dependencies(robot_status_client
  rclcpp
)

rosidl_get_typesupport_target(
  cpp_typesupport_target_client
  ${PROJECT_NAME}
  "rosidl_typesupport_cpp"
)

target_link_libraries(
  robot_status_client
  "${cpp_typesupport_target_client}"
)

# ---------------------------------------------------------
# Robot Move Action Server
# ---------------------------------------------------------

add_executable(
  robot_move_action_server
  src/robot_move_action_server.cpp
)

ament_target_dependencies(
  robot_move_action_server
  rclcpp
  rclcpp_action
)

rosidl_get_typesupport_target(
  cpp_typesupport_target_action
  ${PROJECT_NAME}
  "rosidl_typesupport_cpp"
)

target_link_libraries(
  robot_move_action_server
  "${cpp_typesupport_target_action}"
)

# ---------------------------------------------------------
# Robot Move Action Client
# ---------------------------------------------------------

add_executable(
  robot_move_action_client
  src/robot_move_action_client.cpp
)

ament_target_dependencies(
  robot_move_action_client
  rclcpp
  rclcpp_action
)

rosidl_get_typesupport_target(
  cpp_typesupport_target_action_client
  ${PROJECT_NAME}
  "rosidl_typesupport_cpp"
)

target_link_libraries(
  robot_move_action_client
  "${cpp_typesupport_target_action_client}"
)

# ---------------------------------------------------------
# Install ROS 2 executables
# ---------------------------------------------------------

install(TARGETS
  robot_status_node
  robot_monitor_node
  robot_status_server
  robot_status_client
  robot_move_action_server
  robot_move_action_client
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
