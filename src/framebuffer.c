#include "framebuffer.h"

#include "utils.h"
#include "texture.h"
#include "stdio.h"

GLuint FBO_screen = 0;
GLuint VAO_screen = 0;

GLuint FBO_game = 0;
GLuint FBO_game_texture_color = 0;
GLuint FBO_game_texture_color_ui = 0;
GLuint FBO_game_texture_depth = 0;

void framebuffer_create(int window_w, int window_h)
{
    if (!VAO_screen)
    {
        float vertices[] = {  
            -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 
             1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,   0.0f, 1.0f, 
    
            -1.0f,  1.0f, 0.0f,   0.0f, 1.0f, 
             1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,   1.0f, 1.0f
        };
        
        VAO_screen = opengl_create_vao();
        opengl_create_vbo(vertices, sizeof(vertices));
        opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
        opengl_vbo_layout(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
    }
    
    if (FBO_game)
    {
        glDeleteTextures(1, &FBO_game_texture_color);
        glDeleteTextures(1, &FBO_game_texture_depth);
        glDeleteFramebuffers(1, &FBO_game);
    }
    
    FBO_game = opengl_create_fbo();

    // two color textures, one for main game image and one for
    // ui elements, which are crosshair and block wireframe
    FBO_game_texture_color = framebuffer_color_texture_create(window_w, window_h);
    FBO_game_texture_color_ui = framebuffer_color_texture_create(window_w, window_h);
    FBO_game_texture_depth = framebuffer_depth_texture_create(window_w, window_h);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBO_game_texture_color, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, FBO_game_texture_color_ui, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, FBO_game_texture_depth, 0);
    
    // Enable writing to both color textures
    GLenum bufs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, bufs);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Game framebuffer is incomplete!\n");
    }
}

void framebuffer_bind(GLuint framebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}