#include "texture.h"

#include "string.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "stb_image.h"
#include "config.h"

GLuint texture_blocks;
GLuint texture_skybox_day;
GLuint texture_skybox_evening;
GLuint texture_skybox_night;
GLuint texture_sun;
GLuint texture_moon;

static void exit_if_not_loaded_or_wrong_channels(const char* path, unsigned char* data, 
                                          int channels, int channels_required)
{
    if (!data)
    {
        fprintf(stderr, "%s: failed to open texture\n", path);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    else if (channels != channels_required)
    {
        fprintf(stderr, "%s: expected %d channels in image, but got %d\n", 
                path, channels_required, channels);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
}

static GLuint texture_init(GLuint target)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(target, texture);
    return texture;
}

static void texture_load_from_file(GLuint target, const char* path, int desired_channels)
{
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 0);

    exit_if_not_loaded_or_wrong_channels(path, data, channels, desired_channels);
    
    GLuint format = desired_channels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(target, 0, format, w, h, 0, 
                 format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
}

static GLuint texture_2d_create(const char* path)
{
    GLuint texture = texture_init(GL_TEXTURE_2D);
    texture_load_from_file(GL_TEXTURE_2D, path, 4);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

static GLuint texture_array_create(const char* path)
{
    GLuint texture = texture_init(GL_TEXTURE_2D_ARRAY);

    stbi_set_flip_vertically_on_load(1);
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 0);

    exit_if_not_loaded_or_wrong_channels(path, data, channels, 4);

    int atlas_row_size = w * channels;

    int tiles = 256;
    int tile_w = 16; 
    int tile_h = 16;
    int tile_row_size = tile_w * channels;

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, tile_w, tile_h, 
                 tiles, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // 16 x 16 pixels, 4 bytes per pixel
    unsigned char* tile_data = malloc(tile_w * tile_h * channels);
    
    for (int y = 0; y < 16; y++)
    for (int x = 0; x < 16; x++)
    {
        // Extract tile data from texture atlas
        for (int row = 0; row < tile_h; row++)
        {
            memcpy(tile_data + row * tile_row_size, 
                   data + (x * tile_row_size) + atlas_row_size * (y * tile_h + row), 
                   tile_row_size);
        }

        int curr_tile = x + y * 16;
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, curr_tile, tile_w, 
                        tile_h, 1, GL_RGBA, GL_UNSIGNED_BYTE, tile_data);
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);
    
    // Enable anisotropic filtering if it's available
    if (GLAD_GL_EXT_texture_filter_anisotropic)
    {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, 
                        ANISOTROPIC_FILTER_LEVEL);
    }
    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    stbi_image_free(data);
    free(tile_data);

    return texture;
}

static GLuint texture_skybox_create(const char* paths[6])
{
    GLuint texture = texture_init(GL_TEXTURE_CUBE_MAP);

    stbi_set_flip_vertically_on_load(0);

    for (int i = 0; i < 6; i++)
        texture_load_from_file(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, paths[i], 3);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

void textures_init()
{
    texture_blocks = texture_array_create("textures/blocks.png");

    texture_skybox_day = texture_skybox_create(
        (const char*[6]){
            "textures/skybox/day/right.png",
            "textures/skybox/day/left.png",
            "textures/skybox/day/top.png",
            "textures/skybox/day/bottom.png",
            "textures/skybox/day/front.png",
            "textures/skybox/day/back.png"
        }
    );

    texture_skybox_evening = texture_skybox_create(
        (const char*[6]){
            "textures/skybox/evening/right.png",
            "textures/skybox/evening/left.png",
            "textures/skybox/evening/top.png",
            "textures/skybox/evening/bottom.png",
            "textures/skybox/evening/front.png",
            "textures/skybox/evening/back.png"
        }
    );

    texture_skybox_night = texture_skybox_create(
        (const char*[6]){
            "textures/skybox/night/right.png",
            "textures/skybox/night/left.png",
            "textures/skybox/night/top.png",
            "textures/skybox/night/bottom.png",
            "textures/skybox/night/front.png",
            "textures/skybox/night/back.png"
        }
    );

    texture_sun = texture_2d_create("textures/sun.png");
    texture_moon = texture_2d_create("textures/moon.png");
}

GLuint framebuffer_color_texture_create(int width, int height)
{
    GLuint texture = texture_init(GL_TEXTURE_2D);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return texture;
}

GLuint framebuffer_depth_texture_create(int width, int height)
{
    GLuint texture = texture_init(GL_TEXTURE_2D);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, 
                 height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return texture;
}

void texture_2d_bind(GLuint texture, int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void texture_array_bind(GLuint texture, int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
}

void texture_skybox_bind(GLuint texture, int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
}

static void texture_free(GLuint* texture)
{
    if (*texture) 
    {
        glDeleteTextures(1, texture);
        *texture = 0;
    }
}

void textures_free()
{
    texture_free(&texture_blocks);
    texture_free(&texture_skybox_day);
    texture_free(&texture_skybox_evening);
    texture_free(&texture_skybox_night);
    texture_free(&texture_sun);
    texture_free(&texture_moon);
}