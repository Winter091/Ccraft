#include <renderer/hand_item_2d.h>

#include <assert.h>

#include <glad/glad.h>
#include <cglm/cglm.h>

#include <utils.h>
#include <map/block.h>
#include <map/map.h>
#include <shader.h>
#include <texture.h>
#include <window.h>

typedef struct
{
    GLuint VAO;
    GLuint VBO;
    mat4 model_matrix;

    int current_block;
    float aspect_ratio;

    mat4 camera_mvp;
}
RenderData;

static RenderData s_data;
static int is_initted = 0;


static void regenerate_opengl_buffers(int block)
{
    if (s_data.VAO)
    {
        glDeleteBuffers(1, &s_data.VAO);
        glDeleteVertexArrays(1, &s_data.VAO);
    }

    Vertex vertices[36];
    int faces[6] = {1, 1, 1, 1, 1, 1};
    float ao[6][4] = {0};
    int curr_vertex_count = 0;

    if (block_is_plant(block))
        gen_plant_vertices(vertices, &curr_vertex_count, 0, 0, 0, block, 1.0f);
    else
        gen_cube_vertices(vertices, &curr_vertex_count, 0, 0, 0, block, 1.0f, 0, faces, ao);

    s_data.VAO = opengl_create_vao();
    s_data.VBO = opengl_create_vbo(vertices, curr_vertex_count * sizeof(Vertex));
    opengl_vbo_layout(0, 3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), 0);
    opengl_vbo_layout(1, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), 3 * sizeof(float));
    opengl_vbo_layout(2, 1, GL_FLOAT,         GL_FALSE, sizeof(Vertex), 5 * sizeof(float));
    opengl_vbo_layout(3, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), 6 * sizeof(float));
    opengl_vbo_layout(4, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), 6 * sizeof(float) + 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void regenerate_model_matrix(int block)
{
    glm_mat4_identity(s_data.model_matrix);
    
    if (block == BLOCK_PLAYER_HAND)
    {
        glm_translate(s_data.model_matrix, (vec3){0.3f, -0.3f, 0.34f});
        glm_rotate(s_data.model_matrix, -1.0f,  (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(s_data.model_matrix,  0.48f, (vec3){0.0f, 1.0f, 0.0f});
        glm_rotate(s_data.model_matrix, -0.18f, (vec3){0.0f, 0.0f, 1.0f});
        glm_scale(s_data.model_matrix, (vec3){0.125f, 0.3f, 0.1f});
    }
    else if (block_is_plant(block))
    {
        glm_translate(s_data.model_matrix, (vec3){0.11f, -0.08f, 0.77f});
        glm_rotate(s_data.model_matrix, 0.2f,  (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(s_data.model_matrix, 0.5f,  (vec3){0.0f, 1.0f, 0.0f});
        glm_rotate(s_data.model_matrix, 0.12f, (vec3){0.0f, 0.0f, 1.0f});
        glm_scale(s_data.model_matrix, (vec3){0.1f, 0.1f, 0.1f});
    }
    else
    {
        glm_translate(s_data.model_matrix, (vec3){0.14f, -0.13f, 0.73f});
        // glm_rotate(p->model_mat_item, 0.0f, (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(s_data.model_matrix, 1.0f, (vec3){0.0f, 1.0f, 0.0f});
        // glm_rotate(p->model_mat_item, 0.0f, (vec3){0.0f, 0.0f, 1.0f});
        glm_scale(s_data.model_matrix, (vec3){0.1f, 0.1f, 0.1f});
    }

    // Matrix multuplication order is reversed in code, so it's 
    // the first operation what's below.
    // Firstly translate current item by (-0.5, -0.5, -0.5) to 
    // put the center of an item into the center of coordinates
    // (for correct rotation)
    glm_translate(s_data.model_matrix, (vec3){-0.5f, -0.5f, -0.5f});
}

static void regenerate_camera_matrices(float aspect_ratio)
{
    // The camera is at (0, 0, -1) and looks at (0, 0, 0)
    mat4 view, projection;
    glm_look((vec3){0.0f, 0.0f, 1.0f}, (vec3){0.0f, 0.0f, -1.0f}, 
             (vec3){0.0f, 1.0f, 0.0f}, view);

    glm_perspective(glm_rad(50.0f), aspect_ratio, 0.01f, 2.0f, projection);

    glm_mat4_mulN((mat4* []){&projection, &view, &s_data.model_matrix}, 3, s_data.camera_mvp);
}

static void regenerate_for_block(int block)
{
    regenerate_opengl_buffers(block);
    regenerate_model_matrix(block);
    regenerate_camera_matrices(s_data.aspect_ratio);
    s_data.current_block = block;
}

static void on_framebuffer_size_change_callback(void* this_object, int new_width, int new_height)
{
    assert(is_initted && "Callback was not unregistered");
    s_data.aspect_ratio = (float)new_width / new_height;
    regenerate_camera_matrices(s_data.aspect_ratio);
}

static void render()
{
    glUseProgram(shader_handitem);
    shader_set_mat4(shader_handitem, "mvp_matrix", s_data.camera_mvp);
    shader_set_texture_array(shader_handitem, "texture_sampler", texture_blocks, 0);
    shader_set_float1(shader_handitem, "block_light", map_get_blocks_light());

    GLboolean old_blend;
    glGetBooleanv(GL_BLEND, &old_blend);
    glDisable(GL_BLEND);

    glBindVertexArray(s_data.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    if (old_blend)
        glEnable(GL_BLEND);
}

void renderer_hand_item_2d_init(float aspect_ratio)
{
    s_data.VAO = 0;
    s_data.VBO = 0;
    glm_mat4_identity(s_data.model_matrix);
    s_data.current_block = -1;
    s_data.aspect_ratio = aspect_ratio;
    regenerate_camera_matrices(aspect_ratio);

    register_framebuffer_size_change_callback(NULL, on_framebuffer_size_change_callback);

    is_initted = 1;
}

void renderer_hand_item_2d_render(int block)
{
    assert(is_initted && "Renderer's init was not called prior to render call");

    if (s_data.current_block != block)
        regenerate_for_block(block);

    render();
}

void renderer_hand_item_2d_free()
{
    if (s_data.VAO)
    {
        glDeleteVertexArrays(1, &s_data.VAO);
        glDeleteBuffers(1, &s_data.VBO);
    }

    is_initted = 0;
}