#include <renderer/block_wireframe.h>


#include <assert.h>

#include <glad/glad.h>
#include <cglm/cglm.h>

#include <utils.h>
#include <shader.h>
#include <config.h>

typedef struct
{
    GLuint VAO;
    GLuint VBO;
}
RenderData;

static RenderData s_data;
static int is_initted = 0;

void renderer_block_wireframe_init()
{
    const float x1 = 0.0f;
    const float y1 = 0.0f;
    const float z1 = 0.0f;
    const float x2 = 1.0f;
    const float y2 = 1.0f;
    const float z2 = 1.0f;
    const float offset = 0.001f * BLOCK_SIZE;

    float vertices_wireframe[] = {
        x1 - offset, y1 - offset, z1 - offset,
        x2 + offset, y1 - offset, z1 - offset,

        x2 + offset, y1 - offset, z1 - offset,
        x2 + offset, y2 + offset, z1 - offset,

        x2 + offset, y2 + offset, z1 - offset,
        x1 - offset, y2 + offset, z1 - offset,

        x1 - offset, y2 + offset, z1 - offset,
        x1 - offset, y1 - offset, z1 - offset,

        x1 - offset, y1 - offset, z2 + offset,
        x2 + offset, y1 - offset, z2 + offset,

        x2 + offset, y1 - offset, z2 + offset,
        x2 + offset, y2 + offset, z2 + offset,

        x2 + offset, y2 + offset, z2 + offset,
        x1 - offset, y2 + offset, z2 + offset,

        x1 - offset, y2 + offset, z2 + offset,
        x1 - offset, y1 - offset, z2 + offset,

        x1 - offset, y1 - offset, z2 + offset,
        x1 - offset, y1 - offset, z1 - offset,

        x2 + offset, y1 - offset, z2 + offset,
        x2 + offset, y1 - offset, z1 - offset,

        x2 + offset, y2 + offset, z2 + offset,
        x2 + offset, y2 + offset, z1 - offset,

        x1 - offset, y2 + offset, z2 + offset,
        x1 - offset, y2 + offset, z1 - offset
    };

    s_data.VAO = opengl_create_vao();
    s_data.VAO = opengl_create_vbo(vertices_wireframe, sizeof(vertices_wireframe));
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    is_initted = 1;
}

void renderer_block_wireframe_render(mat4 cam_vp_matrix, vec3 block_aabb[2])
{
    assert(is_initted && "Renderer's init was not called prior to render call");

    glBindVertexArray(s_data.VAO);

    vec3 scaling;
    glm_vec3_sub(block_aabb[1], block_aabb[0], scaling);

    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, block_aabb[0]);
    glm_scale(model, scaling);

    mat4 mvp;
    glm_mat4_mul(cam_vp_matrix, model, mvp);

    glUseProgram(shader_line);
    shader_set_mat4(shader_line, "mvp_matrix", mvp);
    
    glLineWidth(2);
    glDrawArrays(GL_LINES, 0, 24);
}

void renderer_block_wireframe_free()
{    
    if (s_data.VAO)
    {
        glDeleteVertexArrays(1, &s_data.VAO);
        glDeleteBuffers(1, &s_data.VBO);
    }

    is_initted = 0;
}