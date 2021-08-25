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
    p->is_sneaking = 0;
    p->is_running = 0;

    p->VAO_item = 0;
    p->VBO_item = 0;
    regenerate_item(p);

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
    regenerate_item(p);
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
    int const moving_to_plus_x = (p->cam->speed[0] >= 0);
    
    if (moving_to_plus_x)
        p->cam->pos[0] -= (p->hitbox[1][0] - block_hitbox[0][0]) + 0.00001f;
    else
        p->cam->pos[0] += (block_hitbox[1][0] - p->hitbox[0][0]) + 0.00001f;
    
    p->cam->speed[0] = 0.0f;
}

static void collision_y(Player* p, vec3 block_hitbox[2])
{
    int const moving_to_plus_y = (p->cam->speed[1] >= 0);
    
    if (moving_to_plus_y)
        p->cam->pos[1] -= (p->hitbox[1][1] - block_hitbox[0][1]) + 0.00001f;
    else
    {
        p->cam->pos[1] += (block_hitbox[1][1] - p->hitbox[0][1]) + 0.00001f;
        p->on_ground = 1;
    }

    p->cam->speed[1] = 0.0f;
}

static void collision_z(Player* p, vec3 block_hitbox[2])
{
    int const moving_to_plus_z = (p->cam->speed[2] >= 0);
    
    if (moving_to_plus_z)
        p->cam->pos[2] -= (p->hitbox[1][2] - block_hitbox[0][2]) + 0.00001f;
    else
        p->cam->pos[2] += (block_hitbox[1][2] - p->hitbox[0][2]) + 0.00001f;

    p->cam->speed[2] = 0.0f;
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
    if (magnitude < 0.00001f)
        return;
    
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

static void decelerate(Player* p, vec3 frame_accel)
{
    vec3 accel;

    glm_vec3_copy(p->cam->speed, accel);
    accel[1] = 0.0f;
    glm_vec3_normalize(accel);
    glm_vec3_scale(accel, -1.0f, accel);
    glm_vec3_scale(accel, DECELERATION_HORIZONTAL, accel);

    glm_vec3_add(frame_accel, accel, frame_accel);
}

static void accelerate_wasd(Player* p, vec3 frame_accel)
{
    int const key_w = window_is_key_pressed(GLFW_KEY_W);
    int const key_s = window_is_key_pressed(GLFW_KEY_S);
    int const key_a = window_is_key_pressed(GLFW_KEY_A);
    int const key_d = window_is_key_pressed(GLFW_KEY_D);

    // generate front & right vectors
    vec3 front, right;
    front[0] = cosf(glm_rad(p->cam->yaw));
    front[1] = 0.0f;
    front[2] = sinf(glm_rad(p->cam->yaw));
    glm_vec3_normalize(front);
    glm_vec3_crossn(front, p->cam->up, right);

    vec3 accel = {0.0f};
    
    if (key_w || key_s)
    {
        vec3 a = {front[0], 0.0f, front[2]};
        if (key_s) glm_vec3_negate(a);

        glm_vec3_add(accel, a, accel);
    }

    if (key_a || key_d)
    {
        vec3 a = {right[0], 0.0f, right[2]};
        if (key_a) glm_vec3_negate(a);
        
        glm_vec3_add(accel, a, accel);
    }

    float s = ACCELERATION_HORIZONTAL;
    if (!p->on_ground && !p->in_water)
        s = ACCELERATION_HORIZONTAL / 6.0f;
    glm_vec3_scale(accel, s, accel);

    glm_vec3_add(frame_accel, accel, frame_accel);
}

static void gen_motion_vector_walk(Player* p, double dt, vec3 res)
{
    int const key_w     = window_is_key_pressed(GLFW_KEY_W);
    int const key_s     = window_is_key_pressed(GLFW_KEY_S);
    int const key_a     = window_is_key_pressed(GLFW_KEY_A);
    int const key_d     = window_is_key_pressed(GLFW_KEY_D);
    int const key_space = window_is_key_pressed(GLFW_KEY_SPACE);
    int const key_shift = window_is_key_pressed(GLFW_KEY_LEFT_SHIFT);
    int const key_ctrl  = window_is_key_pressed(GLFW_KEY_LEFT_CONTROL);

    vec3 frame_accel = {0.0f};
    vec3 frame_speed = {0.0f};
    static int done_decelerating_sneak = 0;
    static int done_decelerating_run = 0;
    int decelerated_no_keys = 0;

    p->is_sneaking = key_shift;
    p->is_running = key_ctrl;

    if (p->is_sneaking && p->is_running)
        p->is_running = 0;

    float xz_speed = glm_vec2_norm((vec2){ p->cam->speed[0], 
                                   p->cam->speed[2] });

    if (p->is_sneaking && xz_speed > MAX_SNEAK_SPEED 
        && !done_decelerating_sneak && p->on_ground)
    {
        decelerate(p, frame_accel);
    }
    else if (!p->is_running && xz_speed > MAX_MOVE_SPEED
             && !done_decelerating_run && p->on_ground)
    {
        decelerate(p, frame_accel);
    }
    else if (key_w || key_s || key_a || key_d)
    {
        accelerate_wasd(p, frame_accel);
    }
    else if (p->on_ground || p->in_water)
    {
        decelerate(p, frame_accel);
        decelerated_no_keys = 1;
    }

    // Sneak & run logic
    if (!done_decelerating_sneak && xz_speed <= MAX_SNEAK_SPEED)
        done_decelerating_sneak = 1;
    else if (done_decelerating_sneak && !key_shift)
        done_decelerating_sneak = 0;
    
    if (!done_decelerating_run && xz_speed <= MAX_MOVE_SPEED)
        done_decelerating_run = 1;
    else if (done_decelerating_run && p->is_running)
        done_decelerating_run = 0;

    // Gravity, jumps
    if (p->in_water)
        frame_accel[1] -= GRAVITY_WATER;
    else
        frame_accel[1] -= GRAVITY;
    
    if (key_space)
    {
        if (p->on_ground)
            frame_speed[1] = JUMP_POWER;
        else if (p->in_water)
            frame_accel[1] += ACCELERATION_WATER_EMERGE;
    }

    // Calculate frame speed, add it to camera's speed
    vec3 speed_from_accel;
    glm_vec3_copy(frame_accel, speed_from_accel);
    glm_vec3_scale(speed_from_accel, dt, speed_from_accel);
    glm_vec3_add(speed_from_accel, frame_speed, frame_speed);

    vec3 old_cam_speed;
    glm_vec3_copy(p->cam->speed, old_cam_speed);
    glm_vec3_add(frame_speed, p->cam->speed, p->cam->speed);

    // Stop player completely if he was decelerating and
    // speed has changed its direction almost by 180 degrees
    float angle = glm_vec3_angle(
        (vec3){p->cam->speed[0], 0.0f, p->cam->speed[2]},
        (vec3){old_cam_speed[0], 0.0f, old_cam_speed[2]});
    
    float const margin = GLM_PIf / 16.0f;
    if (angle > GLM_PIf - margin && angle < GLM_PIf + margin
        && decelerated_no_keys)
    {
        glm_vec3_zero(p->cam->speed);
    }

    // Clamp horizontal speed
    float max_xz_speed;
    if (p->in_water)
        max_xz_speed = MAX_SWIM_SPEED;
    else if (p->is_running || !done_decelerating_run)
        max_xz_speed = MAX_RUN_SPEED;
    else if (p->is_sneaking && done_decelerating_sneak)
        max_xz_speed = MAX_SNEAK_SPEED;
    else
        max_xz_speed = MAX_MOVE_SPEED;

    xz_speed = glm_vec2_norm((vec2){ p->cam->speed[0], 
                             p->cam->speed[2] });
    if (xz_speed > max_xz_speed)
    {
        float const s = max_xz_speed / xz_speed;
        p->cam->speed[0] *= s;
        p->cam->speed[2] *= s;
    }

    // Clamp vertical speed
    if (p->in_water)
    {
        if (p->cam->speed[1] > MAX_EMERGE_SPEED)
            p->cam->speed[1] = MAX_EMERGE_SPEED;
        else if (p->cam->speed[1] < -MAX_DIVE_SPEED)
            p->cam->speed[1] = -MAX_DIVE_SPEED;
    }
    else
    {
        if (p->cam->speed[1] < -MAX_FALL_SPEED)
            p->cam->speed[1] = -MAX_FALL_SPEED;
    }

    // Calculate frame motion, add it to camera's position
    vec3 frame_motion;
    glm_vec3_copy(p->cam->speed, frame_motion);
    glm_vec3_scale(frame_motion, dt, frame_motion);
    glm_vec3_copy(frame_motion, res);
}

static void gen_motion_vector_fly(Player* p, double dt, vec3 res)
{   
    int const key_w     = window_is_key_pressed(GLFW_KEY_W);
    int const key_s     = window_is_key_pressed(GLFW_KEY_S);
    int const key_a     = window_is_key_pressed(GLFW_KEY_A);
    int const key_d     = window_is_key_pressed(GLFW_KEY_D);
    int const key_shift = window_is_key_pressed(GLFW_KEY_LEFT_SHIFT);
    int const key_ctrl  = window_is_key_pressed(GLFW_KEY_LEFT_CONTROL);

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
    
    glm_vec3_copy(total_move, p->cam->speed);
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
    // Render item using additional camera created here;
    // The camera is at (0, 0, -1) and looks at (0, 0, 0)
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
}

void player_save(Player* p)
{
    db_insert_player_info(p);
}

void player_destroy(Player* p)
{
    player_save(p);
    free(p->cam);
    free(p);
}