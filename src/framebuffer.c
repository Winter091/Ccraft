#include "framebuffer.h"

#include "stdio.h"

#include "utils.h"
#include "texture.h"
#include "window.h"

static void fb_callback(void* this_object, int new_width, int new_height)
{
    Framebuffers* fb = (Framebuffers*)this_object;
    framebuffers_rebuild(fb, new_width, new_height);
}

static void create_gbuf(Framebuffers* fb, int window_w, int window_h)
{
    fb->gbuf_fbo = opengl_create_fbo();
    
    fb->gbuf_tex_color        = framebuffer_color_texture_create(window_w, window_h);
    fb->gbuf_tex_color_ui     = framebuffer_color_texture_create(window_w, window_h);
    fb->gbuf_tex_color_pass_1 = framebuffer_color_texture_create(window_w, window_h);
    fb->gbuf_tex_depth        = framebuffer_depth_texture_create(window_w, window_h);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb->gbuf_tex_color, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fb->gbuf_tex_color_ui, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fb->gbuf_tex_color_pass_1, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, fb->gbuf_tex_depth, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Framebuffer is incomplete!\n");
}

static void delete_gbuf(Framebuffers* fb)
{
    glDeleteTextures(4, (GLuint[])
    {
        fb->gbuf_tex_color,
        fb->gbuf_tex_color_ui,
        fb->gbuf_tex_color_pass_1,
        fb->gbuf_tex_depth
    });
    glDeleteFramebuffers(1, &fb->gbuf_fbo);
}

Framebuffers* framebuffers_create(int window_w, int window_h)
{
    Framebuffers* fb = malloc(sizeof(Framebuffers));

    register_framebuffer_size_change_callback(fb, fb_callback);

    // 0 is default-created fbo which is 
    // bound to glfw window
    fb->default_fbo = 0;

    static float screen_vertices[] = {
        //     pos             tex_coord
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f, 

        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f, 
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f
    };
    
    fb->quad_vao = opengl_create_vao();
    fb->quad_vbo = opengl_create_vbo(screen_vertices, sizeof(screen_vertices));
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    opengl_vbo_layout(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));  

    create_gbuf(fb, window_w, window_h);

    fb->gbuf_shadow_near = opengl_create_fbo();

    fb->near_shadowmap_w = 2048;
    fb->gbuf_shadow_near_map = framebuffer_depth_texture_create(fb->near_shadowmap_w, fb->near_shadowmap_w);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, fb->gbuf_shadow_near_map, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Near Shadow Framebuffer is incomplete!\n");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    fb->gbuf_shadow_far = opengl_create_fbo();

    fb->far_shadowmap_w = 2048;
    fb->gbuf_shadow_far_map = framebuffer_depth_texture_create(fb->far_shadowmap_w, fb->far_shadowmap_w);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, fb->gbuf_shadow_far_map, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Far Shadow Framebuffer is incomplete!\n");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return fb;
}

void framebuffers_rebuild(Framebuffers* fb, int new_window_w, int new_window_h)
{
    delete_gbuf(fb);
    create_gbuf(fb, new_window_w, new_window_h);
}

void framebuffers_destroy(Framebuffers* fb)
{
    delete_gbuf(fb);
    glDeleteBuffers(1, &fb->quad_vbo);
    glDeleteVertexArrays(1, &fb->quad_vao);

    free(fb);
}

void framebuffer_use(Framebuffers* fb, FbType type)
{
    switch (type)
    {
        case FBTYPE_DEFAULT:
            glBindFramebuffer(GL_FRAMEBUFFER, fb->default_fbo);
            break;
        case FBTYPE_TEXTURE:
            glBindFramebuffer(GL_FRAMEBUFFER, fb->gbuf_fbo);
            break;
        case FBTYPE_SHADOW_NEAR:
            glBindFramebuffer(GL_FRAMEBUFFER, fb->gbuf_shadow_near);
            break;
        case FBTYPE_SHADOW_FAR:
            glBindFramebuffer(GL_FRAMEBUFFER, fb->gbuf_shadow_far);
            break;
        default:
            printf("How did we get here?\n");
    }
}

void framebuffer_use_texture(FbTextureType type)
{
    switch (type)
    {
        case TEX_COLOR:
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glClearBufferfv(GL_COLOR, 0, (const GLfloat[]){1.0f, 0.0f, 0.0f, 1.0f});
            break;
        case TEX_UI:
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            glClearBufferfv(GL_COLOR, 0, (const GLfloat[]){0.0f, 1.0f, 0.0f, 0.0f});
            break;
        case TEX_PASS_1:
            glDrawBuffer(GL_COLOR_ATTACHMENT2);
            glClearBufferfv(GL_COLOR, 0, (const GLfloat[]){1.0f, 0.0f, 1.0f, 1.0f});
            break;
        default:
            printf("How dod we get here?\n");
    }
}