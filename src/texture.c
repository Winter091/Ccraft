#include "texture.h"

#include "stb_image.h"
#include "string.h"
#include "config.h"

GLuint texture_blocks;
GLuint texture_skybox;

GLuint texture_create(const char* path)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    stbi_set_flip_vertically_on_load(1);
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 0);

    if (!data)
    {
        fprintf(stderr, "Failed to load image: %s\n", path);
        return 0;
    }

    if (channels != 4)
    {
        fprintf(stderr, "Expected 4 channels in image: %s\n", path);
        return 0;
    }
    
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

    if (!data)
    {
        fprintf(stderr, "Failed to load image: %s\n", path);
        return 0;
    }

    if (channels != 4)
    {
        fprintf(stderr, "Expected 4 channels in image: %s\n", path);
        return 0;
    }

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
        if (!data)
        {
            fprintf(stderr, "Failed to load image: %s\n", paths[i]);
            return 0;
        }

        if (channels != 3)
        {
            fprintf(stderr, "Expected 3 channels in image: %s\n", paths[i]);
            return 0;
        }

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
        "textures/minecraft_blocks.png"
    );
    if (!texture_blocks)
    {
        fprintf(stderr, "Texture was not loaded!\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    texture_skybox = skybox_texture_create(
        (const char*[6]){
            "textures/skybox/right.png",
            "textures/skybox/left.png",
            "textures/skybox/top.png",
            "textures/skybox/bottom.png",
            "textures/skybox/front.png",
            "textures/skybox/back.png"
        }
    );
    if (!texture_skybox)
    {
        fprintf(stderr, "Texture was not loaded!\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
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