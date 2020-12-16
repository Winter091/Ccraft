cmake_minimum_required(VERSION 3.10)

# set project name
project(Ccraft)

file(GLOB SOURCE_FILES src/*.c)
add_executable(
    Ccraft 
    ${SOURCE_FILES} 
    deps/glad-0.1.34/src/glad.c
    deps/stb_image-2.26/src/stb_image.c
    deps/sqlite-3.34.0/src/sqlite3.c
)

target_compile_options(Ccraft PRIVATE -Wall -std=c99)

# add deps to the build process
add_subdirectory(deps/glfw-3.3.2)
add_subdirectory(deps/cglm-0.7.8)

# specify include directories
target_include_directories(Ccraft PRIVATE deps/glfw-3.3.2/include)
target_include_directories(Ccraft PRIVATE deps/glad-0.1.34/include)
target_include_directories(Ccraft PRIVATE deps/cglm-0.7.8/include)
target_include_directories(Ccraft PRIVATE deps/stb_image-2.26/include)
target_include_directories(Ccraft PRIVATE deps/sqlite-3.34.0/include)

# link static libs
target_link_libraries(Ccraft glfw ${GLFW_LIBRARIES})
target_link_libraries(Ccraft cglm)

# copy shaders dir to build folder
add_custom_command(
    TARGET Ccraft POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:Ccraft>/shaders
)

# copy textures dir to build folder
add_custom_command(
    TARGET Ccraft POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/textures $<TARGET_FILE_DIR:Ccraft>/textures
)

# copy maps dir to build folder
add_custom_command(
    TARGET Ccraft POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/maps $<TARGET_FILE_DIR:Ccraft>/maps
)
