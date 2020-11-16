cmake_minimum_required(VERSION 3.10)

# set project name
project(Ccraft)

# folder with all sources
file(GLOB SOURCE_FILES src/*.c)

# include directories
include_directories(deps/GLFW/include/)
include_directories(deps/GLAD/include/)

# add executables
add_executable(Ccraft ${SOURCE_FILES})

# statically link libs
target_link_libraries(Ccraft ${CMAKE_SOURCE_DIR}/deps/GLFW/lib/libglfw3.a)
target_link_libraries(Ccraft ${CMAKE_SOURCE_DIR}/deps/GLAD/lib/libglad.a)