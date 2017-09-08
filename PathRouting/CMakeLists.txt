cmake_minimum_required(VERSION 3.0)
project(path_routing)

###############################################################################
## file globbing ##############################################################
###############################################################################

# these instructions search the directory tree when cmake is
# invoked and put all files that match the pattern in the variables 
# `sources` and `data`
file(GLOB_RECURSE sources *.cpp src/main/*.h)
file(GLOB_RECURSE sources_test *.cpp)
file(GLOB_RECURSE data resources/*)
# you can use set(sources src/main.cpp) etc if you don't want to
# use globing to find files automatically

###############################################################################
## target definitions #########################################################
###############################################################################

# add the data to the target, so it becomes visible in some IDE
add_executable(path_routing ${sources} ${data})

# just for example add some compiler flags
target_compile_options(path_routing PUBLIC -std=c++1y -Wall -Wfloat-conversion -g)


# this copies all resource files in the build directory
# we need this, because we want to work with paths relative to the executable
#file(COPY ${data} DESTINATION resources)

###############################################################################
## dependencies ###############################################################
###############################################################################

# this defines the variables Boost_LIBRARIES that contain all library names
# that we need to link to
find_package(Boost REQUIRED)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(OpenGL REQUIRED)

target_link_libraries(path_routing PUBLIC
  ${Boost_LIBRARIES}
  ${OPENGL_LIBRARIES}
   # here you can add any library dependencies
)

MESSAGE(${Boost_INCLUDE_DIRS})
# this lets me include files relative to the root src dir with a <> pair
target_include_directories(path_routing PUBLIC ${Boost_INCLUDE_DIRS} ../pugixml-1.2/src)

# Add the include directories for the Qt 5 Widgets module to
# the compile lines.
include_directories(${Qt5Widgets_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS})

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
install(TARGETS path_routing DESTINATION example_destination)
# this is basically a repeat of the file copy instruction that copies the
# resources in the build directory, but here we tell cmake that we want it
# in the package
install(DIRECTORY resources DESTINATION example_destination)

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


set(CMAKE_CXX_FLAGS "-O0 -g -std=c++14")
#set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")
#set(CMAKE_CXX_FLAGS_RELEASE "")
