#include <player/player_controller.h>

#include <stdlib.h>
#include <assert.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <map/map.h>
#include <map/block.h>
#include <utils.h>
#include <window.h>
#include <time_measure.h>
#include <player/player_physics.h>


static void on_left_mouse_button(PlayerController* pc)
{
    if (!pc->player->pointing_at_block)
        return;

    Player* p = pc->player;

    int x = p->block_pointed_at[0];
    int y = p->block_pointed_at[1];
    int z = p->block_pointed_at[2];

    map_set_block(x, y, z, BLOCK_AIR);
    p->pointing_at_block = 0;
}

static void find_best_spot_to_place_block(Player* player, int x, int y, int z, int* best_x, 
                                          int* best_y, int* best_z, float* best_dist)
{
    float player_x = blocked(player->pos[0]);
    float player_y = blocked(player->pos[1]);
    float player_z = blocked(player->pos[2]);
    
    if (block_ray_intersection(player->pos, player->front, x, y, z, BLOCK_AIR) 
        && !block_is_solid(map_get_block(x, y, z)))
    {
        float dist = block_player_dist2(x, y, z, player_x, player_y, player_z);
        if (dist < *best_dist)
        {
            *best_dist = dist;
            *best_x = x;
            *best_y = y;
            *best_z = z;
        }
    }
}

static void on_right_mouse_button(PlayerController* pc)
{
    Player* p = pc->player;

    if (!p->pointing_at_block || p->build_block == BLOCK_PLAYER_HAND)
        return;

    int x = p->block_pointed_at[0];
    int y = p->block_pointed_at[1];
    int z = p->block_pointed_at[2];

    int best_x = 0, best_y = 0, best_z = 0;
    float best_dist = BLOCK_BREAK_RADIUS2 * 2;

    // 6 potential spots around active block
    find_best_spot_to_place_block(p, x - 1, y, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p, x + 1, y, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p, x, y - 1, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p, x, y + 1, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p, x, y, z - 1, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(p, x, y, z + 1, &best_x, &best_y, &best_z, &best_dist);

    if ((best_dist > BLOCK_BREAK_RADIUS2) || best_y < 0 || best_y >= CHUNK_HEIGHT)
        return;

    vec3 block_hitbox[2];
    block_gen_aabb(best_x, best_y, best_z, block_hitbox);
    if (aabb_collide(p->hitbox, block_hitbox))
        return;

    map_set_block(best_x, best_y, best_z, p->build_block);
}

static void pc_on_mouse_button_callback(void* this_object, int glfw_keycode, int glfw_action_code)
{
    if (glfw_action_code != GLFW_PRESS)
        return;
    
    PlayerController* pc = (PlayerController*)this_object;

    if (!pc->is_controlling)
    {
        pc->is_controlling = 1;
        pc->is_first_frame = 1;
        return;
    }

    if (glfw_keycode == GLFW_MOUSE_BUTTON_LEFT)
        on_left_mouse_button(pc);
    if (glfw_keycode == GLFW_MOUSE_BUTTON_RIGHT)
        on_right_mouse_button(pc);
}

static void update_dir(PlayerController* pc)
{
    if (!pc->is_controlling)
        return;
    
    double mouse_x, mouse_y;
    glfwGetCursorPos(g_window->glfw, &mouse_x, &mouse_y);

    if (pc->is_first_frame)
    {
        pc->is_first_frame = 0;
        pc->last_mouse_x = mouse_x;
        pc->last_mouse_y = mouse_y;
        return;
    }

    float dx = mouse_x - pc->last_mouse_x;
    float dy = mouse_y - pc->last_mouse_y;

    pc->last_mouse_x = mouse_x;
    pc->last_mouse_y = mouse_y;

    float new_yaw = pc->player->yaw + dx * pc->mouse_sens;
    float new_pitch = pc->player->pitch - dy * pc->mouse_sens;

    const float max_pitch = 89.99f;
    new_pitch = glm_clamp(new_pitch, -max_pitch, max_pitch);

    const float max_yaw = 360.0f;
    new_yaw = loop_between(new_yaw, 0.0f, max_yaw);

    player_set_viewdir(pc->player, new_pitch, new_yaw);
}

static void pc_on_mouse_scroll_callback(void* this_object, float xoffset, float yoffset)
{
    PlayerController* pc = (PlayerController*)this_object;
    if (!pc->is_controlling)
        return;

    if (yoffset > 0)
        player_set_build_block(pc->player, pc->player->build_block + 1);
    else
        player_set_build_block(pc->player, pc->player->build_block - 1);
}

static void decelerate(Player* p, vec3 frame_accel)
{
    vec3 accel;

    glm_vec3_copy(p->speed, accel);
    accel[1] = 0.0f;
    glm_vec3_normalize(accel);
    glm_vec3_scale(accel, -1.0f, accel);
    glm_vec3_scale(accel, DECELERATION_HORIZONTAL, accel);

    glm_vec3_add(frame_accel, accel, frame_accel);
}

static int pc_is_key_pressed(PlayerController* pc, int glfw_keycode)
{
    return pc->is_controlling && window_is_key_pressed(glfw_keycode);
}

static void accelerate_wasd(PlayerController* pc, vec3 frame_accel)
{
    Player* p = pc->player;
    
    int const key_w = pc_is_key_pressed(pc, GLFW_KEY_W);
    int const key_s = pc_is_key_pressed(pc, GLFW_KEY_S);
    int const key_a = pc_is_key_pressed(pc, GLFW_KEY_A);
    int const key_d = pc_is_key_pressed(pc, GLFW_KEY_D);

    // generate front & right vectors
    vec3 front, right;
    front[0] = cosf(glm_rad(p->yaw));
    front[1] = 0.0f;
    front[2] = sinf(glm_rad(p->yaw));
    glm_vec3_normalize(front);
    glm_vec3_crossn(front, p->up, right);

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

static void gen_motion_vector_walk(PlayerController* pc, double dt, vec3 res)
{
    Player* p = pc->player;

    int const key_w     = pc_is_key_pressed(pc, GLFW_KEY_W);
    int const key_s     = pc_is_key_pressed(pc, GLFW_KEY_S);
    int const key_a     = pc_is_key_pressed(pc, GLFW_KEY_A);
    int const key_d     = pc_is_key_pressed(pc, GLFW_KEY_D);
    int const key_space = pc_is_key_pressed(pc, GLFW_KEY_SPACE);
    int const key_shift = pc_is_key_pressed(pc, GLFW_KEY_LEFT_SHIFT);
    int const key_ctrl  = pc_is_key_pressed(pc, GLFW_KEY_LEFT_CONTROL);

    vec3 frame_accel = {0.0f};
    vec3 frame_speed = {0.0f};
    static int done_decelerating_sneak = 0;
    static int done_decelerating_run = 0;
    int decelerated_no_keys = 0;

    p->is_sneaking = key_shift;
    p->is_running = key_ctrl;

    if (p->is_sneaking && p->is_running)
        p->is_running = 0;

    float xz_speed = glm_vec2_norm((vec2){ p->speed[0], p->speed[2] });

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
        accelerate_wasd(pc, frame_accel);
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
    glm_vec3_copy(p->speed, old_cam_speed);
    glm_vec3_add(frame_speed, p->speed, p->speed);

    // Stop player completely if he was decelerating and
    // speed has changed its direction almost by 180 degrees
    float angle = glm_vec3_angle(
        (vec3){p->speed[0], 0.0f, p->speed[2]},
        (vec3){old_cam_speed[0], 0.0f, old_cam_speed[2]});
    
    float const margin = GLM_PIf / 16.0f;
    if (angle > GLM_PIf - margin && angle < GLM_PIf + margin
        && decelerated_no_keys)
    {
        glm_vec3_zero(p->speed);
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

    xz_speed = glm_vec2_norm((vec2){ p->speed[0], p->speed[2] });
    if (xz_speed > max_xz_speed)
    {
        float const s = max_xz_speed / xz_speed;
        p->speed[0] *= s;
        p->speed[2] *= s;
    }

    // Clamp vertical speed
    if (p->in_water)
    {
        if (p->speed[1] > MAX_EMERGE_SPEED)
            p->speed[1] = MAX_EMERGE_SPEED;
        else if (p->speed[1] < -MAX_DIVE_SPEED)
            p->speed[1] = -MAX_DIVE_SPEED;
    }
    else
    {
        if (p->speed[1] < -MAX_FALL_SPEED)
            p->speed[1] = -MAX_FALL_SPEED;
    }

    // Calculate frame motion, add it to camera's position
    vec3 frame_motion;
    glm_vec3_copy(p->speed, frame_motion);
    glm_vec3_scale(frame_motion, dt, frame_motion);
    glm_vec3_copy(frame_motion, res);
}

static void gen_motion_vector_fly(PlayerController* pc, double dt, vec3 res)
{   
    Player* p = pc->player;

    int const key_w     = pc_is_key_pressed(pc, GLFW_KEY_W);
    int const key_s     = pc_is_key_pressed(pc, GLFW_KEY_S);
    int const key_a     = pc_is_key_pressed(pc, GLFW_KEY_A);
    int const key_d     = pc_is_key_pressed(pc, GLFW_KEY_D);
    int const key_shift = pc_is_key_pressed(pc, GLFW_KEY_LEFT_SHIFT);
    int const key_ctrl  = pc_is_key_pressed(pc, GLFW_KEY_LEFT_CONTROL);

    vec3 front, right, up;

    // generate front, right, up vectors
    glm_vec3_copy(p->front, front);
    glm_vec3_crossn(front, p->up, right);
    glm_vec3_copy(p->up, up);

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
    
    glm_vec3_copy(total_move, p->speed);
    glm_vec3_copy(total_move, res);

    glm_vec3_scale(res, dt * pc->fly_speed, res);
}

static void gen_frame_motion(PlayerController* pc, vec3 out)
{
    if (pc->is_fly_mode)
        gen_motion_vector_fly(pc, dt_get(), out);
    else
        gen_motion_vector_walk(pc, dt_get(), out);
}

static void pc_on_keyboard_key_callback(void* this_object, int glfw_keycode, int glfw_action_code)
{
    if (glfw_action_code != GLFW_PRESS)
        return;

    PlayerController* pc = (PlayerController*)this_object;
    
    switch (glfw_keycode)
    {
        case GLFW_KEY_ESCAPE:
            pc->is_controlling = 0;
            break;
        case GLFW_KEY_TAB:
            pc->is_fly_mode = !pc->is_fly_mode;
            break;
    }
}

void playercontroller_do_control(PlayerController* pc)
{
    update_dir(pc);

    vec3 frame_motion;
    gen_frame_motion(pc, frame_motion);

    if (pc->is_fly_mode)
        glm_vec3_add(pc->player->pos, frame_motion, pc->player->pos);
    else
        player_collide_with_map(pc->player, frame_motion);

    // printf("%.3f %.3f %.3f\n", pc->player->front[0], pc->player->front[1], pc->player->front[2]);
}


PlayerController* playercontroller_create(Player* p)
{
    assert(p);

    PlayerController* pc = malloc(sizeof(PlayerController));
    pc->player = p;
    pc->is_first_frame = 1;
    pc->is_fly_mode = 0;
    pc->is_controlling = 1;
    pc->last_mouse_x = 0.0;
    pc->last_mouse_y = 0.0;
    pc->mouse_sens = 0.1f;
    pc->fly_speed = 20.0f * BLOCK_SIZE;
    
    register_mouse_button_key_press_callback(pc, pc_on_mouse_button_callback);
    register_mouse_scroll_callback(pc, pc_on_mouse_scroll_callback);
    register_keyboard_key_press_callback(pc, pc_on_keyboard_key_callback);

    return pc;
}

void playercontroller_destroy(PlayerController* pc)
{
    free(pc);
}
