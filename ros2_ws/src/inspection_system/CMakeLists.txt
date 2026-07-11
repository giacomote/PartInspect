cmake_minimum_required(VERSION 3.8)
project(inspection_system)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ------------------------
# ----- Dependencies -----
# ------------------------

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(ros_gz_interfaces REQUIRED)
find_package(OpenCV REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(std_msgs REQUIRED)

# ------------------------------------
# ----- Targets Node Compilation -----
# ------------------------------------

# Vision node (camera 1)
add_executable(vision_node src/vision_node.cpp)
ament_target_dependencies(vision_node
  rclcpp
  sensor_msgs
  cv_bridge
  image_transport
  std_msgs
)
target_link_libraries(vision_node ${OpenCV_LIBS})

# Check node (camera 2)
add_executable(check_node src/check_node.cpp)
ament_target_dependencies(check_node
  rclcpp
  sensor_msgs
  cv_bridge
  image_transport
  std_msgs
)
target_link_libraries(check_node ${OpenCV_LIBS})

# Dummy camera node (camera 1 mockup)
add_executable(dummy_camera_node src/dummy_camera_node.cpp)
ament_target_dependencies(dummy_camera_node
  rclcpp
  sensor_msgs
  cv_bridge
  image_transport
  std_msgs
)
target_link_libraries(dummy_camera_node ${OpenCV_LIBS})

# Pusher node
add_executable(pusher_node src/pusher_node.cpp)
ament_target_dependencies(pusher_node
  rclcpp
  std_msgs
)

# Controller node
add_executable(controller_node src/controller_node.cpp)
ament_target_dependencies(controller_node
  rclcpp
  std_msgs
)

# Cleaner node
add_executable(cleaner_node src/cleaner_node.cpp)
ament_target_dependencies(cleaner_node
  rclcpp
  nav_msgs
  ros_gz_interfaces
)

# -------------------------
# ----- Install Rules -----
# -------------------------

install(TARGETS
  vision_node
  check_node
  dummy_camera_node
  pusher_node
  controller_node
  cleaner_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch config worlds
  DESTINATION share/${PROJECT_NAME}
)

# -------------------------
# ----- Testing Rules -----
# -------------------------

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
