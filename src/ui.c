#include "ui.h"

#include "utils.h"
#include "stdlib.h"
#include "shader.h"
#include "config.h"
#include "cglm/cglm.h"
#include "block.h"
#include "chunk.h"
#include "texture.h"

#define _USE_MATH_DEFINES
#include "math.h"

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

    glUseProgram(shader_line);
    mat4 ident;
    glm_mat4_identity(ident);
    shader_set_mat4(shader_line, "mvp_matrix", ident);
    
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

    glUseProgram(shader_line);
    shader_set_mat4(shader_line, "mvp_matrix", mvp);

    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_INVERT);
    glLineWidth(2);
    glDrawArrays(GL_LINES, 0, 24);
    glDisable(GL_COLOR_LOGIC_OP);
}

void ui_render_hand_or_item(UI* ui, Player* p)
{
    // It's easier to regenerate cube buffer every time
    // than to try to send 6 uniform block textures
    Vertex* vertices = malloc(36 * sizeof(Vertex));
    block_gen_vertices_unit_cube(vertices, p->build_block ? p->build_block : BLOCK_PLAYER_HAND);
    
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

    // No block is selected; create hand matrix
    if (p->build_block == BLOCK_AIR)
    {
        glm_translate(model, (vec3){0.528f, -0.506f, -0.159f});
        glm_rotate(model, -1.129f, (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(model, 0.422f, (vec3){0.0f, 1.0f, 0.0f});
        glm_rotate(model, -0.31f, (vec3){0.0f, 0.0f, 1.0f});
        glm_scale(model, (vec3){0.1f, 0.3f, 0.1f});
    }
    // block matrix
    else
    {
        glm_translate(model, (vec3){0.562f, -0.536f, -0.092f});
        glm_rotate(model, 0.132f, (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(model, 0.416f, (vec3){0.0f, 1.0f, 0.0f});
        glm_rotate(model, 0.0f, (vec3){0.0f, 0.0f, 1.0f});
        glm_scale(model, (vec3){0.2f, 0.2f, 0.2f});
    }

    // Item renders using additional camera created here;
    // The camera is at (0, 0, -1) and looks at (0, 0, 0)
    mat4 view, projection;
    glm_look((vec3){0.0f, 0.0f, 1.0f}, (vec3){0.0f, 0.0f, -1.0f}, (vec3){0.0f, 1.0f, 0.0f}, view);
    glm_perspective(glm_rad(50.0f), p->cam->aspect_ratio, 0.001f, 10.0f, projection);

    mat4 mvp;
    glm_mat4_mulN((mat4* []){&projection, &view, &model}, 3, mvp);

    glUseProgram(shader_block);
    shader_set_mat4(shader_block, "mvp_matrix", mvp);
    shader_set_int1(shader_block, "texture_sampler", 0);
    array_texture_bind(texture_blocks, 0);

    // remove fog effect
    shader_set_float3(shader_block, "cam_pos", (vec3){0.0f, 0.0f, 0.0f});
    shader_set_float1(shader_block, "fog_dist", 100000.0f);

    // disable face culling for better glass look
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glEnable(GL_CULL_FACE);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}