cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
# set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "default install path" FORCE )
set(CMAKE_PREFIX_PATH "/usr/local/opt/qt5/")
project(path_routing)

###############################################################################
## file globbing ##############################################################
###############################################################################

# these instructions search the directory tree when cmake is
# invoked and put all files that match the pattern in the variables 
# `sources` and `data`
file(GLOB_RECURSE sources
QuadTree.cpp
GLWidget.cpp
AIXM_file_parser.cpp
main.cpp
P3.cpp
Node.cpp
pathrouting.cpp)

#file(GLOB_RECURSE sources_test *.cpp)
#file(GLOB_RECURSE resources ../Airport_data/)
set(resources ../Airport_data)
# you can use set(sources src/main.cpp) etc if you don't want to
# use globing to find files automatically

###############################################################################
## target definitions #########################################################
###############################################################################



# this copies all resource files in the build directory
# we need this, because we want to work with paths relative to the executable
message(${resources})
file(COPY ${resources} DESTINATION .)

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

set(UI_SOURCES pathrouting.ui)

set(QT5_LIBRARIES Qt5::Widgets Qt5::OpenGL)

qt5_wrap_ui(UI_GENERATED_HEADERS ${UI_SOURCES})


#add_subdirectory(../pugixml pugixml_lib)

# Add the include directories for the Qt 5 Widgets module to
# the compile lines.
# include_directories(${Qt5Widgets_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS})

# add the data to the target, so it becomes visible in some IDE
add_executable(path_routing ${sources} ${data} ${UI_GENERATED_HEADERS})

# just for example add some compiler flags
target_compile_options(path_routing INTERFACE "-Wall -Wfloat-conversion -g3 -O")
# this lets me include files relative to the root src dir with a <> pair
target_include_directories(path_routing PUBLIC
  ${Boost_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  ${GLUT_INCLUDE_DIRS}
  ../pugixml/src
  )



find_library(Pugixml_LIBRARY "pugixml"  "../pugixml/build/")
target_link_libraries(path_routing PUBLIC
  ${Boost_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${QT5_LIBRARIES}
  ${Pugixml_LIBRARY}
   # here you can add any library dependencies
   )

###############################################################################
## testing ####################################################################
###############################################################################

# this is for our testing framework
# we don't add REQUIRED because it's just for testing
# find_package(GTest)

# if(GTEST_FOUND)
#   add_executable(unit_tests ${sources_test} ${sources})

#   # we add this define to prevent collision with the main
#   # this might be better solved by not adding the source with the main to the
#   # testing target
#   target_compile_definitions(unit_tests PUBLIC UNIT_TESTS)

#   # this allows us to use our executable as a link library
#   # therefore we can inherit all compiler options and library dependencies
#   set_target_properties(example PROPERTIES ENABLE_EXPORTS on)

#   target_link_libraries(unit_tests PUBLIC
#     ${GTEST_BOTH_LIBRARIES}
#     example
#   )

#   target_include_directories(unit_tests PUBLIC
#     ${GTEST_INCLUDE_DIRS} # doesn't do anything on Linux
#   )
# endif()

###############################################################################
## packaging ##################################################################
###############################################################################

# all install commands get the same destination. this allows us to use paths
# relative to the executable.
#install(TARGETS path_routing DESTINATION bin/ )
# this is basically a repeat of the file copy instruction that copies the
# resources in the build directory, but here we tell cmake that we want it
# in the package
install(DIRECTORY ../Airport_data DESTINATION .)

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
