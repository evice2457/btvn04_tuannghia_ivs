cmake_minimum_required(VERSION 3.8)
project(ackermann_lifecycle_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(std_msgs REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(ackermann_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

add_executable(minimal_lifecycle_node src/minimal_lifecycle_node.cpp)
ament_target_dependencies(minimal_lifecycle_node
  rclcpp
  rclcpp_lifecycle
  std_msgs
  lifecycle_msgs
  ackermann_msgs
  nav_msgs
  tf2_ros
)

install(TARGETS
  minimal_lifecycle_node
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
