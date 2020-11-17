cmake_minimum_required(VERSION 3.10)

# set project name
project(Ccraft)

# add deps to the build process
add_subdirectory(deps/glfw-3.3.2)
add_subdirectory(deps/glad-0.1.34)

add_definitions(-Wall -std=c99)

# specify include directories
include_directories(deps/glfw-3.3.2/include)
include_directories(deps/glad-0.1.34/include)

file(GLOB SOURCE_FILES src/*.c)
add_executable(Ccraft ${SOURCE_FILES})

# link static libs
target_link_libraries(Ccraft glfw ${GLFW_LIBRARIES})
target_link_libraries(Ccraft glad)

# copy shaders dir to build folder
add_custom_command(
    TARGET Ccraft POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:Ccraft>/shaders)
