#include <player/player.h>

#include <map/block.h>
#include <utils.h>
#include <map/map.h>
#include <shader.h>
#include <texture.h>
#include <db.h>
#include <window.h>


void player_update_hitbox(Player* p)
{
    p->hitbox[0][0] = p->pos[0] - BLOCK_SIZE * 0.3f;
    p->hitbox[0][1] = p->pos[1] - BLOCK_SIZE * 1.625f;
    p->hitbox[0][2] = p->pos[2] - BLOCK_SIZE * 0.3f;
    p->hitbox[1][0] = p->pos[0] + BLOCK_SIZE * 0.3f;
    p->hitbox[1][1] = p->pos[1] + BLOCK_SIZE * 0.175f;
    p->hitbox[1][2] = p->pos[2] + BLOCK_SIZE * 0.3f;
}

static void player_put_on_ground_level(Player* p)
{
    int bx = CHUNK_WIDTH / 2;
    int bz = CHUNK_WIDTH / 2;
    int by = map_get_highest_block(bx, bz);

    p->pos[0] = bx * BLOCK_SIZE + BLOCK_SIZE / 2;
    p->pos[1] = by * BLOCK_SIZE + BLOCK_SIZE * 3;
    p->pos[2] = bz * BLOCK_SIZE + BLOCK_SIZE / 2;
    player_update_hitbox(p);
}

Player* player_create()
{
    Player* p = malloc(sizeof(Player));

    my_glm_vec3_set(p->front, 1.0f, 0.0f, 1.0f);
    my_glm_vec3_set(p->up, 0.0f, 1.0f, 0.0f);
    my_glm_vec3_set(p->speed, 0.0f, 0.0f, 0.0f);

    p->yaw = 0.0f;
    p->pitch = 0.0f;

    p->build_block = BLOCK_PLAYER_HAND;
    p->pointing_at_block = 0;
    my_glm_ivec3_set(p->block_pointed_at, 0, 0, 0);

    glm_vec3_fill(p->pos, 0.0f);
    int is_player_in_db = db_has_player_info();
    if (is_player_in_db)
        db_load_player_info(p);

    map_force_chunks_near_player(p->pos);

    if (!is_player_in_db)
        player_put_on_ground_level(p);

    player_update_hitbox(p);

    p->on_ground = 0;
    p->in_water = 0;
    p->is_sneaking = 0;
    p->is_running = 0;

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
            
            if (block_ray_intersection(p->pos, p->front, x, y, z, block))
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
}

static void check_if_in_water(Player* p)
{
    int player_x = (int)blocked(p->pos[0]);
    int player_y = (int)blocked(p->pos[1]);
    int player_z = (int)blocked(p->pos[2]);

    for (int y = -1; y <= 1; y++)
    {
        int by = player_y + y;
        
        if (by < 0 || by >= CHUNK_HEIGHT)
            continue;
        
        for (int x = -1; x <= 1; x++)
        for (int z = -1; z <= 1; z++)
        {
            int bx = player_x + x;
            int bz = player_z + z;
            
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

void player_save(Player* p)
{
    db_save_player_info(p);
}

void player_destroy(Player* p)
{
    player_save(p);
    free(p);
}