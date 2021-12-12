#include <player/player.h>

#include <map/block.h>
#include <utils.h>
#include <map/map.h>
#include <shader.h>
#include <texture.h>
#include <db.h>
#include <window.h>

// Update VAO & VBO to match current build block
static void regenerate_item_buffer(Player* p)
{
    if (p->VAO_item)
    {
        glDeleteBuffers(1, &p->VBO_item);
        glDeleteVertexArrays(1, &p->VAO_item);
    }

    Vertex* vertices = malloc(36 * sizeof(Vertex));
    int faces[6] = {1, 1, 1, 1, 1, 1};
    float ao[6][4] = {0};
    int curr_vertex_count = 0;

    if (block_is_plant(p->build_block))
    {
        gen_plant_vertices(vertices, &curr_vertex_count, 
                           0, 0, 0, p->build_block, 1.0f);
    }
    else
    {
        gen_cube_vertices(vertices, &curr_vertex_count, 0, 0, 0, 
                          p->build_block, 1.0f, 0, faces, ao);
    }

    p->VAO_item = opengl_create_vao();
    p->VBO_item = opengl_create_vbo(vertices, curr_vertex_count * sizeof(Vertex));
    opengl_vbo_layout(0, 3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), 0);
    opengl_vbo_layout(1, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), 3 * sizeof(float));
    opengl_vbo_layout(2, 1, GL_FLOAT,         GL_FALSE, sizeof(Vertex), 5 * sizeof(float));
    opengl_vbo_layout(3, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), 6 * sizeof(float));
    opengl_vbo_layout(4, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), 6 * sizeof(float) + 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    free(vertices);
}

static void regenerate_item_model_matrix(Player* p)
{
    glm_mat4_identity(p->model_mat_item);
    
    if (p->build_block == BLOCK_PLAYER_HAND)
    {
        glm_translate(p->model_mat_item, (vec3){0.3f, -0.3f, 0.34f});
        glm_rotate(p->model_mat_item, -1.0f,  (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(p->model_mat_item,  0.48f, (vec3){0.0f, 1.0f, 0.0f});
        glm_rotate(p->model_mat_item, -0.18f, (vec3){0.0f, 0.0f, 1.0f});
        glm_scale(p->model_mat_item, (vec3){0.125f, 0.3f, 0.1f});
    }
    else if (block_is_plant(p->build_block))
    {
        glm_translate(p->model_mat_item, (vec3){0.11f, -0.08f, 0.77f});
        glm_rotate(p->model_mat_item, 0.2f,  (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(p->model_mat_item, 0.5f,  (vec3){0.0f, 1.0f, 0.0f});
        glm_rotate(p->model_mat_item, 0.12f, (vec3){0.0f, 0.0f, 1.0f});
        glm_scale(p->model_mat_item, (vec3){0.1f, 0.1f, 0.1f});
    }
    else
    {
        glm_translate(p->model_mat_item, (vec3){0.14f, -0.13f, 0.73f});
        //glm_rotate(p->model_mat_item, 0.0f, (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(p->model_mat_item, 1.0f, (vec3){0.0f, 1.0f, 0.0f});
        //glm_rotate(p->model_mat_item, 0.0f, (vec3){0.0f, 0.0f, 1.0f});
        glm_scale(p->model_mat_item, (vec3){0.1f, 0.1f, 0.1f});
    }

    // Matrix multuplication order is reversed in code, so it's 
    // the first operation what's below.
    // Firstly translate current item by (-0.5, -0.5, -0.5) to 
    // put the center of an item into the center of coordinates
    // (for correct rotation)
    glm_translate(p->model_mat_item, (vec3){-0.5f, -0.5f, -0.5f});
}

static void regenerate_item(Player* p)
{
    regenerate_item_buffer(p);
    regenerate_item_model_matrix(p);
}

void player_update_hitbox(Player* p)
{
    p->hitbox[0][0] = p->pos[0] - BLOCK_SIZE * 0.3f;
    p->hitbox[0][1] = p->pos[1] - BLOCK_SIZE * 1.625f;
    p->hitbox[0][2] = p->pos[2] - BLOCK_SIZE * 0.3f;
    p->hitbox[1][0] = p->pos[0] + BLOCK_SIZE * 0.3f;
    p->hitbox[1][1] = p->pos[1] + BLOCK_SIZE * 0.175f;
    p->hitbox[1][2] = p->pos[2] + BLOCK_SIZE * 0.3f;
}

Player* player_create()
{
    Player* p = malloc(sizeof(Player));

    // TODO: Think about starting values
    my_glm_vec3_set(p->front, 1.0f, 0.0f, 1.0f);
    my_glm_vec3_set(p->up, 0.0f, 1.0f, 0.0f);
    my_glm_vec3_set(p->speed, 0.0f, 0.0f, 0.0f);

    p->yaw = 0.0f;
    p->pitch = 0.0f;

    p->build_block = BLOCK_PLAYER_HAND;
    p->pointing_at_block = 0;
    my_glm_ivec3_set(p->block_pointed_at, 0, 0, 0);

    glm_vec3_fill(p->pos, 0.0f);
    int db_has_entry = db_has_player_info();
    if (db_has_entry)
        db_load_player_info(p);

    map_force_chunks_near_player(p->pos);

    // Put player on ground level
    if (!db_has_entry)
    {
        int bx = CHUNK_WIDTH / 2;
        int bz = CHUNK_WIDTH / 2;
        int by = map_get_highest_block(bx, bz);
        p->pos[0] = bx * BLOCK_SIZE + BLOCK_SIZE / 2;
        p->pos[1] = by * BLOCK_SIZE + BLOCK_SIZE * 3;
        p->pos[2] = bz * BLOCK_SIZE + BLOCK_SIZE / 2;
    }

    player_update_hitbox(p);

    p->on_ground = 0;
    p->in_water = 0;
    p->is_sneaking = 0;
    p->is_running = 0;

    p->VAO_item = 0;
    p->VBO_item = 0;
    regenerate_item(p);

    return p;
}

// TODO: Move to camera class
static void update_block_pointing_at(Player* p)
{
    float cam_x = blocked(p->pos[0]);
    float cam_y = blocked(p->pos[1]);
    float cam_z = blocked(p->pos[2]);
    int icam_x = (int)cam_x;
    int icam_y = (int)cam_y;
    int icam_z = (int)cam_z;

    int found = 0;
    int best_x = 0, best_y = 0, best_z = 0;
    float best_dist = BLOCK_BREAK_RADIUS2 * 2;

    // Iterate over each block around player
    for (int y = icam_y - BLOCK_BREAK_RADIUS; y <= icam_y + BLOCK_BREAK_RADIUS; y++)
    {
        if (y < 0 || y >= CHUNK_HEIGHT) continue;

        for (int x = icam_x - BLOCK_BREAK_RADIUS; x <= icam_x + BLOCK_BREAK_RADIUS; x++)
        for (int z = icam_z - BLOCK_BREAK_RADIUS; z <= icam_z + BLOCK_BREAK_RADIUS; z++)
        {
            if (block_player_dist2(x, y, z, cam_x, cam_y, cam_z) > BLOCK_BREAK_RADIUS2)
                continue;
            
            unsigned char block = map_get_block(x, y, z);
            if (!block_is_solid(block))
                continue;
            
            if (camera_looks_at_block(p->pos, p->front, x, y, z, block))
            {
                float distance = block_player_dist2(x, y, z, cam_x, cam_y, cam_z);
                if (distance < best_dist)
                {
                    best_dist = distance;
                    best_x = x;
                    best_y = y;
                    best_z = z;
                    found = 1;
                }
            }
        }
    }

    if (!found)
    {
        p->pointing_at_block = 0;
        return;
    }

    p->pointing_at_block = 1;
    my_glm_ivec3_set(p->block_pointed_at, best_x, best_y, best_z);
}

void player_set_build_block(Player* p, int new_block)
{
    // Skip BLOCK_AIR
    if (new_block == BLOCK_AIR)
    {
        if (new_block < p->build_block)
            new_block--;
        else
            new_block++;
    }

    new_block %= BLOCKS_AMOUNT;
    if (new_block < 0) 
        new_block += BLOCKS_AMOUNT;

    if (new_block == BLOCK_AIR)
        new_block++;

    p->build_block = new_block;
    regenerate_item(p);
}

static void check_if_in_water(Player* p)
{
    // TODO: Fix namings
    int cam_x = (int)blocked(p->pos[0]);
    int cam_y = (int)blocked(p->pos[1]);
    int cam_z = (int)blocked(p->pos[2]);

    for (int y = -1; y <= 1; y++)
    {
        int by = cam_y + y;
        
        if (by < 0 || by >= CHUNK_HEIGHT)
            continue;
        
        for (int x = -1; x <= 1; x++)
        for (int z = -1; z <= 1; z++)
        {
            int bx = cam_x + x;
            int bz = cam_z + z;
            
            unsigned char block = map_get_block(bx, by, bz);
            if (block != BLOCK_WATER)
                continue;

            vec3 block_hitbox[2];
            block_gen_aabb(bx, by, bz, block_hitbox);

            if (aabb_collide(p->hitbox, block_hitbox))
            {
                p->in_water = 1;
                return;
            }
        }
    }
    

    p->in_water = 0;
}

void player_set_viewdir(Player* p, float pitch, float yaw)
{
    p->pitch = pitch;
    p->yaw = yaw;

    p->front[0] = cosf(glm_rad(yaw)) * cosf(glm_rad(pitch));
    p->front[1] = sinf(glm_rad(pitch));
    p->front[2] = sinf(glm_rad(yaw)) * cosf(glm_rad(pitch));
    glm_vec3_normalize(p->front);
}

void player_update(Player* p, double dt)
{
    check_if_in_water(p);
    update_block_pointing_at(p);
}

void player_render_item(Player* p)
{
    // TODO:  Restore item rendering
    return;
    // Render item using additional camera created here;
    // The camera is at (0, 0, -1) and looks at (0, 0, 0)
    /*
    mat4 view, projection;
    glm_look((vec3){0.0f, 0.0f, 1.0f}, (vec3){0.0f, 0.0f, -1.0f}, 
             (vec3){0.0f, 1.0f, 0.0f}, view);
    glm_perspective(glm_rad(50.0f), p->cam->aspect_ratio, 0.01f, 2.0f, projection);

    mat4 mvp;
    glm_mat4_mulN((mat4* []){&projection, &view, &p->model_mat_item}, 3, mvp);

    glUseProgram(shader_handitem);
    shader_set_mat4(shader_handitem, "mvp_matrix", mvp);
    shader_set_texture_array(shader_handitem, "texture_sampler", texture_blocks, 0);
    shader_set_float1(shader_handitem, "block_light", map_get_blocks_light());

    glDisable(GL_BLEND);
    glBindVertexArray(p->VAO_item);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    */
}

void player_save(Player* p)
{
    db_save_player_info(p);
}

void player_destroy(Player* p)
{
    player_save(p);
    free(p);
}