cmake_minimum_required(VERSION 3.3)
project(Ccraft LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED YES)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type was specified, building Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

# glfw
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glfw-3.3.2)

# cglm
set(CGLM_SHARED   OFF CACHE BOOL "" FORCE)
set(CGLM_STATIC    ON CACHE BOOL "" FORCE)
set(CGLM_USE_C99   ON CACHE BOOL "" FORCE)
set(CGLM_USE_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/cglm-0.8.3)

add_executable(
    Ccraft

    ${CMAKE_SOURCE_DIR}/deps/glad-0.1.34/src/glad.c
    ${CMAKE_SOURCE_DIR}/deps/stb_image-2.26/src/stb_image.c
    ${CMAKE_SOURCE_DIR}/deps/sqlite-3.34.0/src/sqlite3.c
    ${CMAKE_SOURCE_DIR}/deps/ini-0.1.1/src/ini.c
    ${CMAKE_SOURCE_DIR}/deps/tinycthread-1.2.0/src/tinycthread.c

    ${CMAKE_SOURCE_DIR}/src/camera/camera_controller.c
    ${CMAKE_SOURCE_DIR}/src/camera/camera.c
    ${CMAKE_SOURCE_DIR}/src/map/block.c
    ${CMAKE_SOURCE_DIR}/src/map/chunk.c
    ${CMAKE_SOURCE_DIR}/src/map/map.c
    ${CMAKE_SOURCE_DIR}/src/map/thread_worker.c
    ${CMAKE_SOURCE_DIR}/src/player/player_controller.c
    ${CMAKE_SOURCE_DIR}/src/player/player_physics.c
    ${CMAKE_SOURCE_DIR}/src/player/player.c
    ${CMAKE_SOURCE_DIR}/src/renderer/block_wireframe.c
    ${CMAKE_SOURCE_DIR}/src/renderer/crosshair.c
    ${CMAKE_SOURCE_DIR}/src/renderer/hand_item_2d.c
    ${CMAKE_SOURCE_DIR}/src/config.c
    ${CMAKE_SOURCE_DIR}/src/db.c
    ${CMAKE_SOURCE_DIR}/src/fastnoiselite_impl.c
    ${CMAKE_SOURCE_DIR}/src/framebuffer.c
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/noise_generator.c
    ${CMAKE_SOURCE_DIR}/src/shader.c
    ${CMAKE_SOURCE_DIR}/src/texture.c
    ${CMAKE_SOURCE_DIR}/src/time_measure.c
    ${CMAKE_SOURCE_DIR}/src/utils.c
    ${CMAKE_SOURCE_DIR}/src/window.c
    ${CMAKE_SOURCE_DIR}/src/worldgen.c
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Ccraft PRIVATE DEBUG)
endif()

# Enable warnings
if (MSVC)
    target_compile_options(Ccraft PRIVATE /W3)
else()
    target_compile_options(Ccraft PRIVATE -Wall -Wno-unused-result -Wimplicit)
endif()

# Link static libs
target_link_libraries(Ccraft glfw cglm)

# Add include directories
target_include_directories(
    Ccraft PRIVATE

    ${CMAKE_SOURCE_DIR}/deps/glfw-3.3.2/include
    ${CMAKE_SOURCE_DIR}/deps/cglm-0.8.3/include
    ${CMAKE_SOURCE_DIR}/deps/glad-0.1.34/include
    ${CMAKE_SOURCE_DIR}/deps/stb_image-2.26/include
    ${CMAKE_SOURCE_DIR}/deps/sqlite-3.34.0/include
    ${CMAKE_SOURCE_DIR}/deps/fastnoise-1.0.1/include
    ${CMAKE_SOURCE_DIR}/deps/ini-0.1.1/include
    ${CMAKE_SOURCE_DIR}/deps/tinycthread-1.2.0/include

    ${CMAKE_SOURCE_DIR}/src
)

# # Copy shaders folder to build folder
# add_custom_command(
#     TARGET Ccraft POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:Ccraft>/shaders
# )

# # Copy textures folder to build folder
# add_custom_command(
#     TARGET Ccraft POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/textures $<TARGET_FILE_DIR:Ccraft>/textures
# )

# # Copy maps folder to build folder
# add_custom_command(
#     TARGET Ccraft POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/maps $<TARGET_FILE_DIR:Ccraft>/maps
# )
