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
    GLuint VAO_block_wireframe;
    GLuint VBO_block_wireframe;
}
UI;

static UI* ui;

void ui_init(float aspect_ratio)
{
    ui = malloc(sizeof(UI));

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
    
    glDeleteVertexArrays(1, (GLuint[1]){  ui->VAO_block_wireframe });
    
    glDeleteBuffers(1, (GLuint[2]){ ui->VBO_block_wireframe });

    free(ui);
    ui = NULL;
}
