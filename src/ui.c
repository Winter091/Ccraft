#include "ui.h"

#include "utils.h"
#include "stdlib.h"
#include "shader.h"
#include "config.h"
#include "cglm/cglm.h"

// Create vbo's for ui elements, init shaders
UI* ui_create(float aspect_ratio)
{
    UI* ui = malloc(sizeof(UI));

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
    glEnableVertexAttribArray(0);

    // Block wireframe buffer
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float bs = BLOCK_SIZE;

    float offset = 0.001f * BLOCK_SIZE;
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
    glEnableVertexAttribArray(0);

    // Line shader
    ui->line_shader = create_shader_program(
        "shaders/line_vertex.glsl",
        "shaders/line_fragment.glsl"
    );

    return ui;
}

void ui_update_aspect_ratio(UI* ui, float new_ratio)
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
    glEnableVertexAttribArray(0);
}

void ui_render_crosshair(UI* ui)
{
    glBindVertexArray(ui->VAO_crosshair);

    glUseProgram(ui->line_shader);
    mat4 ident;
    glm_mat4_identity(ident);
    shader_set_mat4(ui->line_shader, "mvp_matrix", ident);
    
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_INVERT);
    glLineWidth(4);
    glDrawArrays(GL_LINES, 0, 4);
    glDisable(GL_COLOR_LOGIC_OP);
}

void ui_render_block_wireframe(UI* ui, Camera* cam)
{
    glBindVertexArray(ui->VAO_block_wireframe);

    float x = cam->active_block[0] * BLOCK_SIZE;
    float y = cam->active_block[1] * BLOCK_SIZE;
    float z = cam->active_block[2] * BLOCK_SIZE;

    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, (vec3){ x, y, z });

    mat4 mvp;
    glm_mat4_mul(cam->vp_matrix, model, mvp);

    glUseProgram(ui->line_shader);
    shader_set_mat4(ui->line_shader, "mvp_matrix", mvp);

    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_INVERT);
    glLineWidth(2);
    glDrawArrays(GL_LINES, 0, 24);
    glDisable(GL_COLOR_LOGIC_OP);
}