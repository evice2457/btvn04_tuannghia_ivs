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
find_package(std_srvs REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(ackermann_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(minimal_lifecycle_node src/minimal_lifecycle_node.cpp)
target_include_directories(minimal_lifecycle_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(minimal_lifecycle_node
  rclcpp
  rclcpp_lifecycle
  std_msgs
  lifecycle_msgs
  ackermann_msgs
  nav_msgs
  tf2_ros
  geometry_msgs
)
add_executable(ackermann_kinematics_node src/ackermann_kinematics_node.cpp)
target_include_directories(ackermann_kinematics_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
ament_target_dependencies(ackermann_kinematics_node
  rclcpp rclcpp_lifecycle ackermann_msgs std_msgs
  nav_msgs geometry_msgs tf2_ros lifecycle_msgs
)
install(TARGETS ackermann_kinematics_node
  DESTINATION lib/${PROJECT_NAME}
)

add_executable(ackermann_system_manager src/ackermann_system_manager.cpp)
target_include_directories(ackermann_system_manager PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(ackermann_system_manager
  rclcpp
  lifecycle_msgs
  std_msgs
  std_srvs
)
install(TARGETS ackermann_system_manager
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS
  minimal_lifecycle_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_ackermann_kinematics test/test_ackermann_kinematics.cpp)
  target_include_directories(test_ackermann_kinematics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
