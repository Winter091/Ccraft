#include "player.h"

#include "block.h"
#include "utils.h"
#include "map.h"
#include "db.h"
#include "limits.h"

Player* player_create(vec3 pos, vec3 dir)
{
    Player* p = malloc(sizeof(Player));

    p->cam = camera_create(pos, dir);
    p->build_block = BLOCK_STONE;

    p->pointing_at_block = 0;
    p->block_pointed_at[0] = 0;
    p->block_pointed_at[1] = 0;
    p->block_pointed_at[2] = 0;

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