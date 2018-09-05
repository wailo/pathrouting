cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_PREFIX_PATH "/usr/local/opt/qt5/")
project(path_routing)

###############################################################################
## file globbing ##############################################################
###############################################################################

# these instructions search the directory tree when cmake is
# invoked and put all files that match the pattern in the variables 
# `sources` and `data`
file(GLOB sources
src/QuadTree.cpp
src/GLWidget.cpp
src/AIXM_file_parser.cpp
src/main.cpp
src/P3.cpp
src/Node.cpp
src/pathrouting.cpp)

file(GLOB XML_FILES Airport_data)
  
###############################################################################
## dependencies ###############################################################
###############################################################################

 # this defines the variables Boost_LIBRARIES that contain all library names
# that we need to link to
find_package(Boost REQUIRED)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(OpenGL REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(UI_SOURCES src/pathrouting.ui)

set(QT5_LIBRARIES Qt5::Widgets Qt5::OpenGL)

qt5_wrap_ui(UI_GENERATED_HEADERS ${UI_SOURCES})

add_subdirectory(pugixml)

###############################################################################
## target definitions #########################################################
###############################################################################

  message("******************************")
  message("${PROJECT_SOURCE_DIR}")
  message("${PROJECT_BINARY_DIR}")
  message("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  message("${CMAKE_BINARY_DIR}")

# add the data to the target, so it becomes visible in some IDE
add_executable(path_routing ${sources} ${UI_GENERATED_HEADERS} ${XML_FILES})

# just for example add some compiler flags
target_compile_options(path_routing INTERFACE "-Wall -Wfloat-conversion -g3 -O")

# this lets me include files relative to the root src dir with a <> pair
target_include_directories(path_routing PUBLIC
  ${Boost_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  ${GLUT_INCLUDE_DIRS})


# find_library(Pugixml_LIBRARY "pugixml"  "pugixml")
target_link_libraries(path_routing PUBLIC
  ${Boost_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${QT5_LIBRARIES}
  pugixml
   # here you can add any library dependencies
   )


set_target_properties( path_routing PROPERTIES OUTPUT_NAME_DEBUG path_routing_d)
set_target_properties( path_routing PROPERTIES OUTPUT_NAME_RELEASE path_routing)

set_target_properties( path_routing PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin" )
set_target_properties( path_routing PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin" )

file(COPY ${XML_FILES} DESTINATION ${CMAKE_BINARY_DIR})              

###############################################################################
## packaging ##################################################################
###############################################################################

# all install commands get the same destination. this allows us to use paths
# relative to the executable.
#install(TARGETS path_routing DESTINATION bin/ )
# this is basically a repeat of the file copy instruction that copies the
# resources in the build directory, but here we tell cmake that we want it
# in the package
# install(DIRECTORY Airport_data DESTINATION .)

# now comes everything we need, to create a package
# there are a lot more variables you can set, and some
# you need to set for some package types, but we want to
# be minimal here
set(CPACK_PACKAGE_NAME "PathRouting")
set(CPACK_PACKAGE_VERSION "1.0.0")

# we don't want to split our program up into several things
set(CPACK_MONOLITHIC_INSTALL 1)


# This must be last
include(CPack)
