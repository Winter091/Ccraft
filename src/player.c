#include "player.h"

#include "block.h"
#include "utils.h"
#include "map.h"
#include "limits.h"
#include "shader.h"
#include "texture.h"
#include "db.h"

static void regenerate_item_buffer(Player* p)
{
    // If already has buffer
    if (p->VAO_item)
    {
        glDeleteBuffers(1, &p->VBO_item);
        glDeleteVertexArrays(1, &p->VAO_item);
    }

    Vertex* vertices = malloc(36 * sizeof(Vertex));
    int faces[6] = {1, 1, 1, 1, 1, 1};
    float ao[6][4] = {0};

    block_gen_vertices(
            vertices, 0, 0, 0, 0,
            p->build_block ? p->build_block : BLOCK_PLAYER_HAND,
            1, 2.0f, faces, ao
    );

    p->VAO_item = opengl_create_vao();
    p->VBO_item = opengl_create_vbo(vertices, 36 * sizeof(Vertex));
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    opengl_vbo_layout(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 3 * sizeof(float));
    opengl_vbo_layout(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), 5 * sizeof(float));
    opengl_vbo_layout(2, 1, GL_UNSIGNED_BYTE, GL_FALSE,  sizeof(Vertex), 6 * sizeof(float));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    free(vertices);
}

Player* player_create()
{
    Player* p = malloc(sizeof(Player));

    p->cam = camera_create();
    p->build_block = BLOCK_STONE;

#if USE_DATABASE
    // overwrite some parameters
    db_get_player_info(p);
#endif

    p->pointing_at_block = 0;
    p->block_pointed_at[0] = 0;
    p->block_pointed_at[1] = 0;
    p->block_pointed_at[2] = 0;

    p->VAO_item = 0;
    p->VBO_item = 0;
    regenerate_item_buffer(p);

    return p;
}

static void update_block_pointing_at(Player* p)
{
    // position of camera in blocks
    int cam_x = p->cam->pos[0] / BLOCK_SIZE;
    int cam_y = p->cam->pos[1] / BLOCK_SIZE;
    int cam_z = p->cam->pos[2] / BLOCK_SIZE;

    // best_* will store the block,
    // if it's found
    int best_x, best_y, best_z;
    int best_dist = INT_MAX;

    // iterate over each block around player
    for (int y = cam_y - BLOCK_BREAK_RADIUS; y <= cam_y + BLOCK_BREAK_RADIUS; y++)
    {
        if (y < 0 || y >= CHUNK_HEIGHT) continue;

        for (int x = cam_x - BLOCK_BREAK_RADIUS; x <= cam_x + BLOCK_BREAK_RADIUS; x++)
            for (int z = cam_z - BLOCK_BREAK_RADIUS; z <= cam_z + BLOCK_BREAK_RADIUS; z++)
            {
                if (block_player_dist2(x, y, z, cam_x, cam_y, cam_z) > BLOCK_BREAK_RADIUS * BLOCK_BREAK_RADIUS)
                    continue;
                
                if (camera_looks_at_block(p->cam, x, y, z))
                {
                    unsigned char block = map_get_block(x, y, z);
                    if (block == BLOCK_AIR)
                        continue;
                    
                    // we need closest block that camera is pointing to
                    int distance = block_player_dist2(cam_x, cam_y, cam_z, x, y, z);
                    if (distance < best_dist)
                    {
                        best_dist = distance;
                        best_x = x;
                        best_y = y;
                        best_z = z;
                    }
                }
            }
    }
    
    if (best_dist == INT_MAX)
    {
        p->pointing_at_block = 0;
        return;
    }

    p->pointing_at_block = 1;
    p->block_pointed_at[0] = best_x;
    p->block_pointed_at[1] = best_y;
    p->block_pointed_at[2] = best_z;
}

void player_set_build_block(Player* p, int new_block)
{
    new_block %= BLOCKS_AMOUNT;
    if (new_block < 0) 
        new_block += BLOCKS_AMOUNT;
    
    p->build_block = new_block;
    regenerate_item_buffer(p);
}

void player_update(Player* p, GLFWwindow* window, double dt)
{
    camera_update(p->cam, window, dt);
    update_block_pointing_at(p);
}

void player_handle_left_mouse_click(Player* p)
{
    if (!p->pointing_at_block)
        return;

    int x = p->block_pointed_at[0];
    int y = p->block_pointed_at[1];
    int z = p->block_pointed_at[2];

    // if camera looks at block nearby, remove the block
    map_set_block(x, y, z, BLOCK_AIR);
    p->pointing_at_block = 0;

#if USE_DATABASE
    // store block change in database
    db_insert_block(
        chunked(x), chunked(z),
        blocked(x), y, blocked(z),
        BLOCK_AIR
    );
#endif
}

static void find_best_spot_to_place_block(
    Camera* cam, int x, int y, int z, 
    int* best_x, int* best_y, int* best_z, int* best_dist
)
{
    // position of camera in blocks
    int cam_x = cam->pos[0] / BLOCK_SIZE;
    int cam_y = cam->pos[1] / BLOCK_SIZE;
    int cam_z = cam->pos[2] / BLOCK_SIZE;
    
    if (camera_looks_at_block(cam, x, y, z) && map_get_block(x, y, z) == BLOCK_AIR)
    {
        int dist = block_player_dist2(cam_x, cam_y, cam_z, x, y, z);
        if (dist < *best_dist)
        {
            *best_dist = dist;
            *best_x = x;
            *best_y = y;
            *best_z = z;
        }
    }
}

void player_handle_right_mouse_click(Player* p)
{
    if (!p->pointing_at_block)
        return;

    int x = p->block_pointed_at[0];
    int y = p->block_pointed_at[1];
    int z = p->block_pointed_at[2];

    int best_x = 0, best_y = 0, best_z = 0;
    int best_dist = INT_MAX;

    // 6 potential spots around active block
    find_best_spot_to_place_block(p->cam, x - 1, y, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p->cam, x + 1, y, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p->cam, x, y - 1, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p->cam, x, y + 1, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p->cam, x, y, z - 1, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p->cam, x, y, z + 1, &best_x, &best_y, &best_z, &best_dist);

    if (best_dist == INT_MAX)
        return;
    else if (best_y < 0 || best_y >= CHUNK_HEIGHT)
        return;

    map_set_block(best_x, best_y, best_z, p->build_block);

#if USE_DATABASE
    // store block change in database
    db_insert_block(
        chunked(best_x), chunked(best_z),
        blocked(best_x), best_y, blocked(best_z),
        p->build_block
    );
#endif
}

void player_render_item(Player* p)
{
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
    glm_perspective(glm_rad(50.0f), p->cam->aspect_ratio, 0.01f, 2.0f, projection);

    mat4 mvp;
    glm_mat4_mulN((mat4* []){&projection, &view, &model}, 3, mvp);

    glUseProgram(shader_block);
    shader_set_mat4(shader_block, "mvp_matrix", mvp);
    shader_set_texture_2d(shader_block, "texture_sampler", texture_blocks, 0);

    // remove fog effect
    shader_set_float3(shader_block, "cam_pos", (vec3){0.0f, 0.0f, 0.0f});
    shader_set_float1(shader_block, "fog_dist", 100000.0f);

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(p->VAO_item);
    glDrawArrays(GL_TRIANGLES, 0, 36);

}

void player_save(Player* p)
{
    db_insert_player_info(p);
}