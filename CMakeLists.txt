cmake_minimum_required(VERSION 3.0.2)
project(ysc_control)

# Find catkin and required components
find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  roscpp
  std_msgs
)

# First try to find tinyxml2 via catkin (ROS-provided)
# If not found, try system installation
find_package(tinyxml2 QUIET)
if(NOT tinyxml2_FOUND)
  # Fallback to pkg-config method
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(TINYXML2 REQUIRED tinyxml2)
  set(tinyxml2_INCLUDE_DIRS ${TINYXML2_INCLUDE_DIRS})
  set(tinyxml2_LIBRARIES ${TINYXML2_LIBRARIES})
endif()

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS geometry_msgs roscpp std_msgs
  DEPENDS tinyxml2
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${tinyxml2_INCLUDE_DIRS}
)

# Add your executable
add_executable(ysc_control_node src/ysc_control_api.cc src/ysc_control_ros1.cc)
target_link_libraries(ysc_control_node
  ${catkin_LIBRARIES}
  ${tinyxml2_LIBRARIES}
)

# Install targets
install(TARGETS ysc_control_node
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)