#include "framebuffer.h"

#include "stdio.h"

#include "utils.h"
#include "texture.h"

// Keep everything in global variables for easier access
// from different files
GLuint FBO_screen = 0;
GLuint VAO_screen = 0;

GLuint FBO_game = 0;
GLuint FBO_game_texture_color = 0;
GLuint FBO_game_texture_color_ui = 0;
GLuint FBO_game_texture_color_pass_1 = 0;
GLuint FBO_game_texture_depth = 0;

void framebuffer_create(int curr_window_w, int curr_window_h)
{
    // If screen quad wasn't generated yet, do it
    if (!VAO_screen)
    {
        static float screen_vertices[] = {
                //     pos             tex_coord
                -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 
                 1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,   0.0f, 1.0f, 

                -1.0f,  1.0f, 0.0f,   0.0f, 1.0f, 
                 1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
                 1.0f,  1.0f, 0.0f,   1.0f, 1.0f
            };
        
        VAO_screen = opengl_create_vao();
        opengl_create_vbo(screen_vertices, sizeof(screen_vertices));
        opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
        opengl_vbo_layout(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));    
    }

    FBO_game = opengl_create_fbo();

    FBO_game_texture_color = framebuffer_color_texture_create(curr_window_w, curr_window_h);
    FBO_game_texture_color_ui = framebuffer_color_texture_create(curr_window_w, curr_window_h);
    FBO_game_texture_color_pass_1 = framebuffer_color_texture_create(curr_window_w, curr_window_h);
    FBO_game_texture_depth = framebuffer_depth_texture_create(curr_window_w, curr_window_h);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBO_game_texture_color, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, FBO_game_texture_color_ui, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, FBO_game_texture_color_pass_1, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, FBO_game_texture_depth, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Game framebuffer is incomplete!\n");
    }
}

void framebuffer_rebuild(int curr_window_w, int curr_window_h)
{
    glDeleteTextures(
        4, (GLuint[])
        {
            FBO_game_texture_color, 
            FBO_game_texture_color_ui, 
            FBO_game_texture_color_pass_1,
            FBO_game_texture_depth
        }
    );
    glDeleteFramebuffers(1, &FBO_game);    
    
    framebuffer_create(curr_window_w, curr_window_h);
}

void framebuffer_bind(GLuint framebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}