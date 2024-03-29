#include <ui.h>

#include <stdlib.h>

#include <utils.h>
#include <shader.h>
#include <config.h>
#include <map/block.h>
#include <map/map.h>
#include <window.h>

typedef struct
{
    GLuint VAO_crosshair;
    GLuint VBO_crosshair;

    GLuint VAO_block_wireframe;
    GLuint VBO_block_wireframe;
}
UI;

static UI* ui;

static void ui_framebuffer_size_change_callback(void* this_object, int new_width, int new_height)
{
    ui_update_aspect_ratio((float)new_width / new_height);
}

void ui_init(float aspect_ratio)
{
    ui = malloc(sizeof(UI));

    register_framebuffer_size_change_callback(ui, ui_framebuffer_size_change_callback);

    // Crosshair buffer
    float center = 0.0f;
    float size = 0.03f;

    float vertices_crosshair[] = {
        (center - size) / aspect_ratio, center, 0.0f,
        (center + size) / aspect_ratio, center, 0.0f,

        center, (center - size), 0.0f,
        center, (center + size), 0.0f
    };
    
    ui->VAO_crosshair = opengl_create_vao();
    ui->VBO_crosshair = opengl_create_vbo(vertices_crosshair, sizeof(vertices_crosshair));
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    // Block wireframe buffer
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float bs = BLOCK_SIZE;

    float offset = 0.001f * BLOCK_SIZE;
    // That's beautiful
    float vertices_wireframe[] = {
        x - offset, y - offset, z - offset,
        x + bs + offset, y - offset, z - offset,

        x + bs + offset, y - offset, z - offset,
        x + bs + offset, y + bs + offset, z - offset,

        x + bs + offset, y + bs + offset, z - offset,
        x - offset, y + bs + offset, z - offset,

        x - offset, y + bs + offset, z - offset,
        x - offset, y - offset, z - offset,

        x - offset, y - offset, z + bs + offset,
        x + bs + offset, y - offset, z + bs + offset,

        x + bs + offset, y - offset, z + bs + offset,
        x + bs + offset, y + bs + offset, z + bs + offset,

        x + bs + offset, y + bs + offset, z + bs + offset,
        x - offset, y + bs + offset, z + bs + offset,

        x - offset, y + bs + offset, z + bs + offset,
        x - offset, y - offset, z + bs + offset,

        x - offset, y - offset, z + bs + offset,
        x - offset, y - offset, z - offset,

        x + bs + offset, y - offset, z + bs + offset,
        x + bs + offset, y - offset, z - offset,

        x + bs + offset, y + bs + offset, z + bs + offset,
        x + bs + offset, y + bs + offset, z - offset,

        x - offset, y + bs + offset, z + bs + offset,
        x - offset, y + bs + offset, z - offset
    };

    ui->VAO_block_wireframe = opengl_create_vao();
    ui->VBO_block_wireframe = opengl_create_vbo(vertices_wireframe, sizeof(vertices_wireframe));
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
}

void ui_update_aspect_ratio(float new_ratio)
{
    glDeleteVertexArrays(1, &ui->VAO_crosshair);
    glDeleteBuffers(1, &ui->VBO_crosshair);

    float center = 0.0f;
    float size = 0.03f;

    float vertices[] = {
        (center - size) / new_ratio, center, 0.0f,
        (center + size) / new_ratio, center, 0.0f,

        center, (center - size), 0.0f,
        center, (center + size), 0.0f
    };

    ui->VAO_crosshair = opengl_create_vao();
    ui->VBO_crosshair = opengl_create_vbo(vertices, sizeof(vertices));
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
}

void ui_render_crosshair()
{
    glBindVertexArray(ui->VAO_crosshair);

    glUseProgram(shader_line);
    mat4 ident;
    glm_mat4_identity(ident);
    shader_set_mat4(shader_line, "mvp_matrix", ident);
    
    glDisable(GL_DEPTH_TEST);
    glLineWidth(4);
    glDrawArrays(GL_LINES, 0, 4);
}

void ui_render_block_wireframe(Player* p, Camera* cam)
{
    glBindVertexArray(ui->VAO_block_wireframe);

    float x = p->block_pointed_at[0] * BLOCK_SIZE;
    float y = p->block_pointed_at[1] * BLOCK_SIZE;
    float z = p->block_pointed_at[2] * BLOCK_SIZE;

    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, (vec3){ x, y, z });

    unsigned char block = map_get_block(p->block_pointed_at[0], 
                                        p->block_pointed_at[1],
                                        p->block_pointed_at[2]);

    // make wireframe smaller
    if (block_is_plant(block))
    {
        glm_scale(model, (vec3){0.5, 0.5f, 0.5f});
        glm_translate(model, (vec3){BLOCK_SIZE / 2, 0.0f, BLOCK_SIZE / 2});
    }

    mat4 mvp;
    glm_mat4_mul(cam->vp_matrix, model, mvp);

    glUseProgram(shader_line);
    shader_set_mat4(shader_line, "mvp_matrix", mvp);
    
    glLineWidth(2);
    glDrawArrays(GL_LINES, 0, 24);
}

void ui_free()
{
    if (ui == NULL)
        return;
    
    glDeleteVertexArrays(2, (GLuint[2]){ ui->VAO_crosshair, 
                                         ui->VAO_block_wireframe });
    
    glDeleteBuffers(2, (GLuint[2]){ ui->VBO_crosshair, 
                                    ui->VBO_block_wireframe });

    free(ui);
    ui = NULL;
}
