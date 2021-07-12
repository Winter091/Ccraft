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

file(GLOB CCRAFT_SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
add_executable(
    Ccraft
    ${CCRAFT_SOURCES}
    ${CMAKE_SOURCE_DIR}/deps/glad-0.1.34/src/glad.c
    ${CMAKE_SOURCE_DIR}/deps/stb_image-2.26/src/stb_image.c
    ${CMAKE_SOURCE_DIR}/deps/sqlite-3.34.0/src/sqlite3.c
    ${CMAKE_SOURCE_DIR}/deps/ini-0.1.1/src/ini.c
    ${CMAKE_SOURCE_DIR}/deps/tinycthread-1.2.0/src/tinycthread.c
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Ccraft PRIVATE DEBUG)
endif()

# Enable warnings
if (MSVC)
    target_compile_options(Ccraft PRIVATE /W3)
else()
    target_compile_options(Ccraft PRIVATE -Wall -Wno-unused-result)
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
)

# # Copy shaders folder to build folder
# add_custom_command(
#     TARGET Ccraft POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:Ccraft>/shaders
# )

# Copy textures folder to build folder
add_custom_command(
    TARGET Ccraft POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/textures $<TARGET_FILE_DIR:Ccraft>/textures
)

# # Copy maps folder to build folder
# add_custom_command(
#     TARGET Ccraft POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/maps $<TARGET_FILE_DIR:Ccraft>/maps
# )
