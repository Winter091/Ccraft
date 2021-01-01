#include "texture.h"

#include "stb_image.h"
#include "string.h"
#include "config.h"
#include "glfw/glfw3.h"

GLuint texture_blocks;
GLuint texture_skybox_day;
GLuint texture_skybox_evening;
GLuint texture_skybox_night;
GLuint texture_sun;
GLuint texture_moon;

void exit_if_not_loaded_or_wrong_channels(const char* path, unsigned char* data, int channels, int channels_required)
{
    if (!data)
    {
        fprintf(stderr, "%s: failed to open texture\n", path);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    else if (channels != channels_required)
    {
        fprintf(
            stderr, "%s: expected %d channels in image, but got %d\n", 
            path, channels_required, channels
        );
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
}


GLuint texture_create(const char* path)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    stbi_set_flip_vertically_on_load(0);
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 0);

    exit_if_not_loaded_or_wrong_channels(path, data, channels, 4);
    
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(data);

    return texture;
}

GLuint array_texture_create(const char* path)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

    stbi_set_flip_vertically_on_load(1);
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 0);

    exit_if_not_loaded_or_wrong_channels(path, data, channels, 4);

    int atlas_row_size = w * channels;

    int tiles = 256;
    int tile_w = 16; 
    int tile_h = 16;
    int tile_row_size = tile_w * channels;

    glTexImage3D(
        GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, tile_w, tile_h, 
        tiles, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
    );

    // 16 x 16 pixels, 4 bytes per pixel
    unsigned char* tile_data = malloc(tile_w * tile_h * channels);
    
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            // extract tile data from texture atlas
            for (int row = 0; row < tile_h; row++)
            {
                memcpy(
                    tile_data + row * tile_row_size, 
                    data + (x * tile_row_size) + atlas_row_size * (y * tile_h + row), 
                    tile_row_size
                );
            }

            int curr_tile = x + y * 16;
            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY, 0, 0, 0, curr_tile, tile_w, 
                tile_h, 1, GL_RGBA, GL_UNSIGNED_BYTE, tile_data
            );
        }
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);
    
    // anisotropic filtering
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, ANISOTROPIC_FILTERING_LEVEL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    stbi_image_free(data);
    free(tile_data);

    return texture;
}

GLuint skybox_texture_create(const char* paths[6])
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    stbi_set_flip_vertically_on_load(0);
    int w, h, channels;

    for (int i = 0; i < 6; i++)
    {
        unsigned char* data = stbi_load(paths[i], &w, &h, &channels, 0);
        exit_if_not_loaded_or_wrong_channels(paths[i], data, channels, 3);

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
            0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data
        );

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

void texture_load()
{
    texture_blocks = array_texture_create(
        "textures/blocks.png"
    );

    texture_skybox_day = skybox_texture_create(
        (const char*[6]){
            "textures/skybox/day/right.png",
            "textures/skybox/day/left.png",
            "textures/skybox/day/top.png",
            "textures/skybox/day/bottom.png",
            "textures/skybox/day/front.png",
            "textures/skybox/day/back.png"
        }
    );

    texture_skybox_evening = skybox_texture_create(
        (const char*[6]){
            "textures/skybox/evening/right.png",
            "textures/skybox/evening/left.png",
            "textures/skybox/evening/top.png",
            "textures/skybox/evening/bottom.png",
            "textures/skybox/evening/front.png",
            "textures/skybox/evening/back.png"
        }
    );

    texture_skybox_night = skybox_texture_create(
        (const char*[6]){
            "textures/skybox/night/right.png",
            "textures/skybox/night/left.png",
            "textures/skybox/night/top.png",
            "textures/skybox/night/bottom.png",
            "textures/skybox/night/front.png",
            "textures/skybox/night/back.png"
        }
    );

    texture_sun = texture_create(
        "textures/sun.png"
    );

    texture_moon = texture_create(
        "textures/moon.png"
    );
}

void texture_bind(GLuint texture, int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void array_texture_bind(GLuint texture, int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
}

void skybox_texture_bind(GLuint texture, int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
}