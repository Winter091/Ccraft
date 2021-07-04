#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include "glad/glad.h"

typedef enum
{
    FBTYPE_DEFAULT,
    FBTYPE_TEXTURE
}
FbType;

typedef enum
{
    TEX_COLOR = 0,
    TEX_UI,
    TEX_PASS_1
}
FbTextureType;

typedef struct
{
    GLuint default_fbo;

    GLuint quad_vao;
    GLuint quad_vbo;

    GLuint gbuf_fbo;
    GLuint gbuf_tex_color;
    GLuint gbuf_tex_color_ui;
    GLuint gbuf_tex_color_pass_1;
    GLuint gbuf_tex_depth;
}
Framebuffers;

Framebuffers* framebuffers_init(int window_w, int window_h);
void framebuffers_rebuild(Framebuffers* fb, int new_window_w, int new_window_h);
void framebuffers_destroy(Framebuffers* fb);

void framebuffer_use(Framebuffers* fb, FbType type);
void framebuffer_use_texture(FbTextureType type);

#endif