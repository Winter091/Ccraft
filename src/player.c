#include "player.h"

#include "block.h"
#include "utils.h"
#include "map.h"
#include "shader.h"
#include "texture.h"
#include "db.h"
#include "window.h"

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
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    opengl_vbo_layout(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 3 * sizeof(float));
    opengl_vbo_layout(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), 5 * sizeof(float));
    opengl_vbo_layout(2, 1, GL_UNSIGNED_BYTE, GL_FALSE,  sizeof(Vertex), 6 * sizeof(float));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    free(vertices);
}

static void update_hitbox(Player* p)
{
    p->hitbox[0][0] = p->cam->pos[0] - BLOCK_SIZE * 0.3f;
    p->hitbox[0][1] = p->cam->pos[1] - BLOCK_SIZE * 1.625f;
    p->hitbox[0][2] = p->cam->pos[2] - BLOCK_SIZE * 0.3f;
    p->hitbox[1][0] = p->cam->pos[0] + BLOCK_SIZE * 0.3f;
    p->hitbox[1][1] = p->cam->pos[1] + BLOCK_SIZE * 0.175f;
    p->hitbox[1][2] = p->cam->pos[2] + BLOCK_SIZE * 0.3f;
}

Player* player_create()
{
    Player* p = malloc(sizeof(Player));

    p->cam = camera_create();
    p->build_block = BLOCK_PLAYER_HAND;

    // overwrite some parameters
    db_get_player_info(p);

    p->pointing_at_block = 0;
    my_glm_ivec3_set(p->block_pointed_at, 0, 0, 0);

    map_force_chunks_near_player(p->cam);

    // If it's newly created world (default pos[1] is -1.0),
    // put player on ground level
    if (p->cam->pos[1] < 0)
    {
        int bx = CHUNK_WIDTH / 2;
        int bz = CHUNK_WIDTH / 2;
        int by = map_get_highest_block(bx, bz);
        p->cam->pos[0] = bx * BLOCK_SIZE + BLOCK_SIZE / 2;
        p->cam->pos[1] = by * BLOCK_SIZE + BLOCK_SIZE * 3;
        p->cam->pos[2] = bz * BLOCK_SIZE + BLOCK_SIZE / 2;
    }

    update_hitbox(p);

    p->on_ground = 0;
    p->in_water = 0;

    p->VAO_item = 0;
    p->VBO_item = 0;
    regenerate_item_buffer(p);

    return p;
}

static void update_block_pointing_at(Player* p)
{
    float cam_x = blocked(p->cam->pos[0]);
    float cam_y = blocked(p->cam->pos[1]);
    float cam_z = blocked(p->cam->pos[2]);
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
            
            if (camera_looks_at_block(p->cam, x, y, z, block))
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
    regenerate_item_buffer(p);
}

// Almost like glm_aabb_aabb(), but >= and <= are replaced with > and <
static inline int aabb_collide(vec3 box[2], vec3 other[2]) 
{
    return (box[0][0] < other[1][0] && box[1][0] > other[0][0])
        && (box[0][1] < other[1][1] && box[1][1] > other[0][1])
        && (box[0][2] < other[1][2] && box[1][2] > other[0][2]);
}

#define FOREACH_SOLID_BLOCK_AROUND()                            \
for (int y = -3; y <= 3; y++)                                   \
{                                                               \
    int by = cam_y + y;                                         \
    if (by < 0 || by >= CHUNK_HEIGHT)                           \
        continue;                                               \
                                                                \
    for (int x = -3; x <= 3; x++)                               \
    for (int z = -3; z <= 3; z++)                               \
    {                                                           \
        int bx = cam_x + x;                                     \
        int bz = cam_z + z;                                     \
                                                                \
        unsigned char block = map_get_block(bx, by, bz);        \
        if (!block_is_solid(block) || block_is_plant(block))    \
            continue;                                           \
                                                                \
        vec3 block_hitbox[2];                                   \
        block_gen_aabb(bx, by, bz, block_hitbox);            

#define FOREACH_SOLID_BLOCK_AROUND_END() }}

static void collision_x(Player* p, vec3 block_hitbox[2])
{
    int const moving_to_plus_x = (p->cam->speed_horizontal[0] >= 0);
    
    if (moving_to_plus_x)
        p->cam->pos[0] -= (p->hitbox[1][0] - block_hitbox[0][0]) + 0.00001f;
    else
        p->cam->pos[0] += (block_hitbox[1][0] - p->hitbox[0][0]) + 0.00001f;
    
    p->cam->speed_horizontal[0] = 0.0;
}

static void collision_y(Player* p, vec3 block_hitbox[2])
{
    int const moving_to_plus_y = (p->cam->speed_vertical >= 0);
    
    if (moving_to_plus_y)
        p->cam->pos[1] -= (p->hitbox[1][1] - block_hitbox[0][1]) + 0.00001f;
    else
    {
        p->cam->pos[1] += (block_hitbox[1][1] - p->hitbox[0][1]) + 0.00001f;
        p->on_ground = 1;
    }

    p->cam->speed_vertical = 0.0f;
}

static void collision_z(Player* p, vec3 block_hitbox[2])
{
    int const moving_to_plus_z = (p->cam->speed_horizontal[1] >= 0);
    
    if (moving_to_plus_z)
        p->cam->pos[2] -= (p->hitbox[1][2] - block_hitbox[0][2]) + 0.00001f;
    else
        p->cam->pos[2] += (block_hitbox[1][2] - p->hitbox[0][2]) + 0.00001f;

    p->cam->speed_horizontal[1] = 0.0;
}

static int collide_one_axis(void (*collision_handler)
                            (Player*, vec3 block_hitbox[2]), Player* p)
{
    int cam_x = (int)blocked(p->cam->pos[0]);
    int cam_y = (int)blocked(p->cam->pos[1]);
    int cam_z = (int)blocked(p->cam->pos[2]);

    update_hitbox(p);

    FOREACH_SOLID_BLOCK_AROUND()
        if (aabb_collide(p->hitbox, block_hitbox))
        {
            collision_handler(p, block_hitbox);
            update_hitbox(p);
            return 1;
        }
    FOREACH_SOLID_BLOCK_AROUND_END()

    return 0;
}

static void collide_all_axes(Player* p, vec3 motion, ivec3 do_collide)
{
    if (do_collide[1])
    {
        p->on_ground = 0;
        p->cam->pos[1] += motion[1];
        do_collide[1] = !collide_one_axis(collision_y, p);
    }
    if (do_collide[0])
    {
        p->cam->pos[0] += motion[0];
        do_collide[0] = !collide_one_axis(collision_x, p);
    }
    if (do_collide[2])
    {
        p->cam->pos[2] += motion[2];
        do_collide[2] = !collide_one_axis(collision_z, p);
    }
}

static void collide_with_map(Player* p, vec3 motion)
{
    float const max_step_size = 0.25f * BLOCK_SIZE;

    float magnitude = glm_vec3_norm(motion);
    int num_steps = 1 + (magnitude / max_step_size);
    float step_size = magnitude / num_steps;

    vec3 step_motion;
    glm_vec3_copy(motion, step_motion);
    glm_vec3_scale(step_motion, (1.0f / magnitude) * step_size, step_motion);

    ivec3 do_collide;
    my_glm_ivec3_set(do_collide, 1, 1, 1);

    for (int i = 0; i < num_steps; i++)
        collide_all_axes(p, step_motion, do_collide);
}

static void gen_motion_vector_walk(Player* p, double dt, vec3 res)
{
    int key_w     = (glfwGetKey(g_window->glfw, GLFW_KEY_W)          == GLFW_PRESS);
    int key_s     = (glfwGetKey(g_window->glfw, GLFW_KEY_S)          == GLFW_PRESS);
    int key_a     = (glfwGetKey(g_window->glfw, GLFW_KEY_A)          == GLFW_PRESS);
    int key_d     = (glfwGetKey(g_window->glfw, GLFW_KEY_D)          == GLFW_PRESS);
    int key_space = (glfwGetKey(g_window->glfw, GLFW_KEY_SPACE)      == GLFW_PRESS);
    int key_shift = (glfwGetKey(g_window->glfw, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

    vec3 front, right, up;

    // generate front, right, up vectors
    front[0] = cosf(glm_rad(p->cam->yaw));
    front[1] = 0.0f;
    front[2] = sinf(glm_rad(p->cam->yaw));
    glm_vec3_normalize(front);
    glm_vec3_crossn(front, p->cam->up, right);
    glm_vec3_copy(p->cam->up, up);

    vec2 frame_speed_horizontal = {0.0f, 0.0f};
    float frame_speed_vertical = 0.0f;

    if (key_w || key_s || key_a || key_d)
    {
        if (key_w || key_s)
        {
            vec2 move = {front[0], front[2]};
            if (key_s) glm_vec2_negate(move);

            glm_vec2_add(frame_speed_horizontal, move, frame_speed_horizontal);
        }

        if (key_a || key_d)
        {
            vec2 move = {right[0], right[2]};
            if (key_a) glm_vec2_negate(move);
            
            glm_vec2_add(frame_speed_horizontal, move, frame_speed_horizontal);
        }
    }
    // No motion keys are pressed at this frame, decelerate
    else
    {
        if (p->on_ground || p->in_water)
        {
            float speed = glm_vec2_norm(p->cam->speed_horizontal);
            // Remove possible div by zero error
            speed += 0.00001f;

            glm_vec2_scale(p->cam->speed_horizontal, 
                           1.0f / (1.0f + (dt * DECELERATION_HORIZONTAL / speed)), 
                           p->cam->speed_horizontal);
        }
    }

    // Jump / emerge from water
    if (key_space)
    {
        if (p->on_ground)
            p->cam->speed_vertical = JUMP_POWER;
        else if (p->in_water)
            p->cam->speed_vertical += JUMP_POWER * dt * 3.0f;
    }

    // Smooth transition from run speed to sneak speed
    float horizontal_speed = glm_vec2_norm(p->cam->speed_horizontal);
    if (key_shift && horizontal_speed > MAX_MOVE_SPEED_SNEAK)
    {
        // Main goal is to decelerate, so don't mind
        // wasd key presses until we're slowed
        glm_vec2_fill(frame_speed_horizontal, 0.0f);
    
        // Remove possible div by zero error
        horizontal_speed += 0.00001f;

        glm_vec2_scale(p->cam->speed_horizontal, 
                       1.0f / (1.0f + (dt * DECELERATION_HORIZONTAL / horizontal_speed)), 
                       p->cam->speed_horizontal);
    }

    if (p->in_water)
        frame_speed_vertical -= GRAVITY / 3.0f;
    else
        frame_speed_vertical -= GRAVITY;

    glm_vec2_scale(frame_speed_horizontal, 
                   dt * ACCELERATION_HORIZONTAL, 
                   frame_speed_horizontal);
    frame_speed_vertical *= dt;
    
    // Apply generated horizontal motion
    glm_vec2_add(p->cam->speed_horizontal, frame_speed_horizontal, 
                 p->cam->speed_horizontal);

    horizontal_speed = glm_vec2_norm(p->cam->speed_horizontal);

    float max_hor_speed = (p->in_water ? MAX_SWIM_SPEED : MAX_MOVE_SPEED);

    if (horizontal_speed > max_hor_speed)
    {
        glm_vec2_scale(p->cam->speed_horizontal, max_hor_speed / horizontal_speed, 
                       p->cam->speed_horizontal);
    }
    else if (horizontal_speed < 0.001f)
    {
        glm_vec2_fill(p->cam->speed_horizontal, 0.0f);
    }

    // Apply and clamp vertical speed
    p->cam->speed_vertical += frame_speed_vertical;
    if (p->in_water)
    {
        if (p->cam->speed_vertical > MAX_EMERGE_SPEED)
            p->cam->speed_vertical = MAX_EMERGE_SPEED;
        else if (p->cam->speed_vertical < -MAX_DIVE_SPEED)
            p->cam->speed_vertical = -MAX_DIVE_SPEED;
    }
    else
    {
        if (p->cam->speed_vertical < -MAX_FALL_SPEED)
            p->cam->speed_vertical = -MAX_FALL_SPEED;
    }

    // Don't move player yet, just save values we need to move with
    res[0] = p->cam->speed_horizontal[0];
    res[1] = p->cam->speed_vertical;
    res[2] = p->cam->speed_horizontal[1];
    glm_vec3_scale(res, dt, res);
}

static void gen_motion_vector_fly(Player* p, double dt, vec3 res)
{   
    int key_w     = glfwGetKey(g_window->glfw, GLFW_KEY_W) == GLFW_PRESS;
    int key_s     = glfwGetKey(g_window->glfw, GLFW_KEY_S) == GLFW_PRESS;
    int key_a     = glfwGetKey(g_window->glfw, GLFW_KEY_A) == GLFW_PRESS;
    int key_d     = glfwGetKey(g_window->glfw, GLFW_KEY_D) == GLFW_PRESS;
    int key_shift = glfwGetKey(g_window->glfw, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    int key_ctrl  = glfwGetKey(g_window->glfw, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

    vec3 front, right, up;

    // generate front, right, up vectors
    glm_vec3_copy(p->cam->front, front);
    glm_vec3_crossn(front, p->cam->up, right);
    glm_vec3_copy(p->cam->up, up);

    vec3 total_move = {0.0f};

    if (key_w)
        glm_vec3_add(total_move, front, total_move);
    if (key_s)
        glm_vec3_sub(total_move, front, total_move);

    if (key_d)
        glm_vec3_add(total_move, right, total_move);
    if (key_a)
        glm_vec3_sub(total_move, right, total_move);

    if (key_shift)
        glm_vec3_add(total_move, up, total_move);
    if (key_ctrl)
        glm_vec3_sub(total_move, up, total_move);
    
    p->cam->speed_horizontal[0] = total_move[0];
    p->cam->speed_vertical = total_move[1];
    p->cam->speed_horizontal[1] = total_move[2];

    glm_vec3_copy(total_move, res);
    glm_vec3_scale(res, dt * p->cam->fly_speed, res);
}

static void check_if_in_water(Player* p)
{
    int cam_x = (int)blocked(p->cam->pos[0]);
    int cam_y = (int)blocked(p->cam->pos[1]);
    int cam_z = (int)blocked(p->cam->pos[2]);

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

void player_update(Player* p, double dt)
{
    if (!p->cam->is_active)
        return;
    
    camera_update_view_dir(p->cam);
    camera_update_parameters(p->cam, dt);
    check_if_in_water(p);

    vec3 motion;
    
    if (p->cam->is_fly_mode)
    {
        gen_motion_vector_fly(p, dt, motion);
        glm_vec3_add(p->cam->pos, motion, p->cam->pos);
    }
    else
    {
        gen_motion_vector_walk(p, dt, motion);
        collide_with_map(p, motion);
    }

    update_block_pointing_at(p);
    camera_update_matrices(p->cam);
}

void player_handle_left_mouse_click(Player* p)
{
    if (!p->pointing_at_block)
        return;

    int x = p->block_pointed_at[0];
    int y = p->block_pointed_at[1];
    int z = p->block_pointed_at[2];

    map_set_block(x, y, z, BLOCK_AIR);
    p->pointing_at_block = 0;
}

// Helper function for player_handle_right_mouse_click()
static void find_best_spot_to_place_block(Camera* cam, int x, int y, int z, int* best_x, 
                                          int* best_y, int* best_z, float* best_dist)
{
    float cam_x = blocked(cam->pos[0]);
    float cam_y = blocked(cam->pos[1]);
    float cam_z = blocked(cam->pos[2]);
    
    if (camera_looks_at_block(cam, x, y, z, BLOCK_AIR) 
        && !block_is_solid(map_get_block(x, y, z)))
    {
        float dist = block_player_dist2(x, y, z, cam_x, cam_y, cam_z);
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
    if (!p->pointing_at_block || p->build_block == BLOCK_PLAYER_HAND)
        return;

    int x = p->block_pointed_at[0];
    int y = p->block_pointed_at[1];
    int z = p->block_pointed_at[2];

    int best_x = 0, best_y = 0, best_z = 0;
    float best_dist = BLOCK_BREAK_RADIUS2 * 2;

    // 6 potential spots around active block
    find_best_spot_to_place_block(p->cam, x - 1, y, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p->cam, x + 1, y, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p->cam, x, y - 1, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p->cam, x, y + 1, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p->cam, x, y, z - 1, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p->cam, x, y, z + 1, &best_x, &best_y, &best_z, &best_dist);

    if ((best_dist > BLOCK_BREAK_RADIUS2) || best_y < 0 || best_y >= CHUNK_HEIGHT)
        return;

    vec3 block_hitbox[2];
    block_gen_aabb(best_x, best_y, best_z, block_hitbox);
    if (aabb_collide(p->hitbox, block_hitbox))
        return;

    map_set_block(best_x, best_y, best_z, p->build_block);
}

void player_render_item(Player* p)
{
    mat4 model;
    glm_mat4_identity(model);

    if (p->build_block == BLOCK_PLAYER_HAND)
    {
        glm_rotate(model, -1.092f, (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(model, 0.336f, (vec3){0.0f, 1.0f, 0.0f});
        glm_rotate(model, -0.154f, (vec3){0.0f, 0.0f, 1.0f});
        glm_scale(model, (vec3){0.1f, 0.3f, 0.1f});
        glm_translate(model, (vec3){2.76f, -1.98f, 0.16f});
    }
    else if (block_is_plant(p->build_block))
    {
        glm_rotate(model, 0.063f, (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(model, 0.364f, (vec3){0.0f, 1.0f, 0.0f});
        glm_rotate(model, 0.000f, (vec3){0.0f, 0.0f, 1.0f});
        glm_scale(model, (vec3){0.1f, 0.1f, 0.1f});
        glm_translate(model, (vec3){-1.98f, -0.80f, 7.01f});
    }
    else
    {
        glm_rotate(model, 0.154f, (vec3){1.0f, 0.0f, 0.0f});
        glm_rotate(model, 0.448f, (vec3){0.0f, 1.0f, 0.0f});
        glm_rotate(model, 0.049f, (vec3){0.0f, 0.0f, 1.0f});
        glm_scale(model, (vec3){0.1f, 0.1f, 0.1f});
        glm_translate(model, (vec3){-2.4f, -0.57f, 6.78f});
    }

    // Render item using additional camera created here;
    // The camera is at (0, 0, -1) and looks at (0, 0, 0)
    mat4 view, projection;
    glm_look((vec3){0.0f, 0.0f, 1.0f}, (vec3){0.0f, 0.0f, -1.0f}, 
             (vec3){0.0f, 1.0f, 0.0f}, view);
    glm_perspective(glm_rad(50.0f), p->cam->aspect_ratio, 0.01f, 2.0f, projection);

    mat4 mvp;
    glm_mat4_mulN((mat4* []){&projection, &view, &model}, 3, mvp);

    glUseProgram(shader_block);
    shader_set_mat4(shader_block, "mvp_matrix", mvp);
    shader_set_texture_array(shader_block, "texture_sampler", texture_blocks, 0);

    // Remove fog effect
    shader_set_float3(shader_block, "cam_pos", (vec3){0.0f, 0.0f, 0.0f});
    shader_set_float1(shader_block, "fog_dist", 100000.0f);

    glDisable(GL_BLEND);
    glBindVertexArray(p->VAO_item);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void player_save(Player* p)
{
    db_insert_player_info(p);
}

void player_exit(Player* p)
{
    player_save(p);
    free(p->cam);
    free(p);
}