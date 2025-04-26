cmake_minimum_required(VERSION 3.0.2)
project(ysc_control)

# Find catkin and required components
find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  roscpp
  std_msgs
)

find_package(tinyxml2 REQUIRED)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS geometry_msgs roscpp std_msgs
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

# Add your executable
add_executable(ysc_control_node src/ysc_control_api.cc src/ysc_control_ros1.cc)
target_link_libraries(ysc_control_node
  ${catkin_LIBRARIES}
  tinyxml2::tinyxml2
)

# Install targets
install(TARGETS ysc_control_node
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
