#include "ui.h"

#include "utils.h"
#include "stdlib.h"
#include "shader.h"
#include "config.h"
#include "cglm/cglm.h"
#include "block.h"
#include "chunk.h"
#include "texture.h"

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

    // Cube buffer
    ui->VAO_cube = opengl_create_vao();
    ui->VBO_cube = opengl_create_vbo_cube();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
    glEnableVertexAttribArray(0);

    // Line shader
    ui->line_shader = create_shader_program(
        "shaders/line_vertex.glsl",
        "shaders/line_fragment.glsl"
    );

    // Cube shader
    ui->cube_shader = create_shader_program(
        "shaders/chunk_vertex.glsl",
        "shaders/chunk_fragment.glsl"
    );

    // Blocks texture
    ui->texture_blocks = array_texture_create(
        "textures/minecraft_blocks.png"
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

void ui_render_block_wireframe(UI* ui, Player* p)
{
    glBindVertexArray(ui->VAO_block_wireframe);

    float x = p->block_pointed_at[0] * BLOCK_SIZE;
    float y = p->block_pointed_at[1] * BLOCK_SIZE;
    float z = p->block_pointed_at[2] * BLOCK_SIZE;

    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, (vec3){ x, y, z });

    mat4 mvp;
    glm_mat4_mul(p->cam->vp_matrix, model, mvp);

    glUseProgram(ui->line_shader);
    shader_set_mat4(ui->line_shader, "mvp_matrix", mvp);

    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_INVERT);
    glLineWidth(2);
    glDrawArrays(GL_LINES, 0, 24);
    glDisable(GL_COLOR_LOGIC_OP);
}

void ui_render_item(UI* ui, Player* p)
{
    Vertex* vertices = malloc(36 * sizeof(Vertex));
    int faces[6] = { 1, 1, 1, 1, 1, 1 };
    float ao[6][4] = {0};
    block_gen_vertices(vertices, 0, 0, 0, 0, p->build_block ? p->build_block : 23, faces, ao);
    
    GLuint VAO = opengl_create_vao();
    GLuint VBO = opengl_create_vbo(vertices, 36 * sizeof(Vertex));

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0
    );
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float))
    );
    glVertexAttribPointer(
        3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(5 * sizeof(float))
    );
    glVertexAttribIPointer(
        2, 1, GL_UNSIGNED_BYTE,  sizeof(Vertex), (void*)(6 * sizeof(float))
    );
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    
    mat4 model;
    glm_mat4_identity(model);

    float sc = 1.0f / BLOCK_SIZE;
    glm_scale(model, (vec3){sc, sc, sc});

    glm_rotate(model, 3.1415f / 10.0f, (vec3){0.0f, 1.0f, 0.0f});

    float tr = -BLOCK_SIZE / 2.0f;
    glm_translate(model, (vec3){tr, tr, tr});
    glm_translate(model, (vec3){0.6f, -0.57f, -0.3f});

    mat4 view;
    glm_look((vec3){0.0f, 0.0f, 1.0f}, (vec3){0.0f, 0.0f, -1.0f}, (vec3){0.0f, 1.0f, 0.0f}, view);

    mat4 projection;
    glm_perspective(glm_rad(50.0f), p->cam->aspect_ratio, 0.001f, 10.0f, projection);

    mat4 mv;
    mat4 mvp;
    glm_mat4_mul(view, model, mv);
    glm_mat4_mul(projection, mv, mvp);

    glUseProgram(ui->cube_shader);
    shader_set_mat4(ui->cube_shader, "mvp_matrix", mvp);
    shader_set_int1(ui->cube_shader, "texture_sampler", 0);
    array_texture_bind(ui->texture_blocks, 0);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}