#include <renderer/crosshair.h>

#include <assert.h>

#include <glad/glad.h>
#include <cglm/cglm.h>

#include <utils.h>
#include <shader.h>
#include <window.h>

typedef struct
{
    GLuint VAO;
    GLuint VBO;
}
RenderData;

static RenderData s_data;
static int is_initted = 0;

static void regenerate_crosshair_data(float aspect_ratio)
{
    if (s_data.VAO)
    {
        glDeleteVertexArrays(1, &s_data.VAO);
        glDeleteBuffers(1, &s_data.VBO);
    }

    float center = 0.0f;
    float size = 0.03f;

    float vertices[] = {
        (center - size) / aspect_ratio, center, 0.0f,
        (center + size) / aspect_ratio, center, 0.0f,

        center, (center - size), 0.0f,
        center, (center + size), 0.0f
    };

    s_data.VAO = opengl_create_vao();
    s_data.VBO = opengl_create_vbo(vertices, sizeof(vertices));
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
}

static void on_framebuffer_size_change_callback(void* this_object, int new_width, int new_height)
{
    assert(is_initted && "Callback was not unregistered");
    const float aspect_ratio = (float)new_width / new_height;
    regenerate_crosshair_data(aspect_ratio);
}

void renderer_crosshair_init(float aspect_ratio)
{
    register_framebuffer_size_change_callback(NULL, on_framebuffer_size_change_callback);

    regenerate_crosshair_data(aspect_ratio);
    is_initted = 1;
}

void renderer_crosshair_render()
{
    assert(is_initted && "Renderer's init was not called prior to render call");

    glBindVertexArray(s_data.VAO);

    glUseProgram(shader_line);
    mat4 identity;
    glm_mat4_identity(identity);
    shader_set_mat4(shader_line, "mvp_matrix", identity);
    GLboolean old_depth_test;
    glGetBooleanv(GL_DEPTH_TEST, &old_depth_test);
    glDisable(GL_DEPTH_TEST);

    glLineWidth(4);
    glDrawArrays(GL_LINES, 0, 4);

    if (old_depth_test)
        glEnable(GL_DEPTH_TEST);
}

void renderer_crosshair_free()
{    
    if (s_data.VAO)
    {
        glDeleteVertexArrays(1, &s_data.VAO);
        glDeleteBuffers(1, &s_data.VBO);
    }

    is_initted = 0;
}