#include <player/player_physics.h>

#include <map/map.h>
#include <map/block.h>
#include <utils.h>

#define FOREACH_SOLID_BLOCK_AROUND(PX, PY, PZ)                  \
for (int y = -3; y <= 3; y++)                                   \
{                                                               \
    int by = PY + y;                                            \
    if (by < 0 || by >= CHUNK_HEIGHT)                           \
        continue;                                               \
                                                                \
    for (int x = -3; x <= 3; x++)                               \
    for (int z = -3; z <= 3; z++)                               \
    {                                                           \
        int bx = PX + x;                                        \
        int bz = PZ + z;                                        \
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
    int const moving_to_plus_x = (p->speed[0] >= 0);
    
    if (moving_to_plus_x)
        p->pos[0] -= (p->hitbox[1][0] - block_hitbox[0][0]) + 0.00001f;
    else
        p->pos[0] += (block_hitbox[1][0] - p->hitbox[0][0]) + 0.00001f;
    
    p->speed[0] = 0.0f;
}

static void collision_y(Player* p, vec3 block_hitbox[2])
{
    int const moving_to_plus_y = (p->speed[1] >= 0);
    
    if (moving_to_plus_y)
        p->pos[1] -= (p->hitbox[1][1] - block_hitbox[0][1]) + 0.00001f;
    else
    {
        p->pos[1] += (block_hitbox[1][1] - p->hitbox[0][1]) + 0.00001f;
        p->on_ground = 1;
    }

    p->speed[1] = 0.0f;
}

static void collision_z(Player* p, vec3 block_hitbox[2])
{
    int const moving_to_plus_z = (p->speed[2] >= 0);
    
    if (moving_to_plus_z)
        p->pos[2] -= (p->hitbox[1][2] - block_hitbox[0][2]) + 0.00001f;
    else
        p->pos[2] += (block_hitbox[1][2] - p->hitbox[0][2]) + 0.00001f;

    p->speed[2] = 0.0f;
}

static int collide_one_axis(void (*collision_handler)
                            (Player*, vec3 block_hitbox[2]), Player* p)
{
    int player_x = (int)blocked(p->pos[0]);
    int player_y = (int)blocked(p->pos[1]);
    int player_z = (int)blocked(p->pos[2]);

    player_update_hitbox(p);

    FOREACH_SOLID_BLOCK_AROUND(player_x, player_y, player_z)
        if (aabb_collide(p->hitbox, block_hitbox))
        {
            collision_handler(p, block_hitbox);
            player_update_hitbox(p);
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
        p->pos[1] += motion[1];
        do_collide[1] = !collide_one_axis(collision_y, p);
    }
    if (do_collide[0])
    {
        p->pos[0] += motion[0];
        do_collide[0] = !collide_one_axis(collision_x, p);
    }
    if (do_collide[2])
    {
        p->pos[2] += motion[2];
        do_collide[2] = !collide_one_axis(collision_z, p);
    }
}

void player_collide_with_map(Player* p, vec3 motion)
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
