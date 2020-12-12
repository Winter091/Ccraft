#include "ui.h"

#include "utils.h"
#include "stdlib.h"
#include "shader.h"
#include "cglm/cglm.h"

UI* ui_create(float aspect_ratio)
{
    float center = 0.0f;
    float size = 0.03f;

    float vertices[] = {
        (center - size) / aspect_ratio, center, 0.0f,
        (center + size) / aspect_ratio, center, 0.0f,

        center, (center - size), 0.0f,
        center, (center + size), 0.0f
    };
    
    UI* ui = malloc(sizeof(UI));

    ui->VAO_crosshair = opengl_create_vao();
    ui->VBO_crosshair = opengl_create_vbo(vertices, sizeof(vertices), 4);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
    glEnableVertexAttribArray(0);

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
    ui->VBO_crosshair = opengl_create_vbo(vertices, sizeof(vertices), 4);

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