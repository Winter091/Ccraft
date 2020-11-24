#include "texture.h"

#include "stb_image.h"
#include "string.h"

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
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    //glGenerateMipmap(GL_TEXTURE_2D);

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

    int texels = 256;
    int texel_w = 16; 
    int texel_h = 16;
    int texel_row_size = texel_w * channels;

    glTexImage3D(
        GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, texel_w, texel_h, 
        texels, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
    );

    // 16 x 16 pixels, 4 bytes per pixel
    unsigned char* texel_data = malloc(texel_w * texel_h * channels);
    
    for (int y = 0; y < 16; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            // extract texel data from texture atlas
            for (int row = 0; row < texel_h; row++)
            {
                memcpy(
                    texel_data + row * texel_row_size, 
                    data + (x * texel_row_size) + atlas_row_size * (y * texel_h + row), 
                    texel_row_size
                );
            }

            int curr_texel = x + y * 16;
            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY, 0, 0, 0, curr_texel, texel_w, 
                texel_h, 1, GL_RGBA, GL_UNSIGNED_BYTE, texel_data
            );
        }
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);
    
    // 16x anisotropic filtering
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    stbi_image_free(data);
    free(texel_data);

    return texture;
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