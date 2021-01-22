#ifndef DB_H_
#define DB_H_

#include "chunk.h"
#include "player.h"

void db_init();

// Store block change
void db_insert_block(int chunk_x, int chunk_z, int x, int y, int z, int block);

// Load all block changes for the given chunk
void db_get_blocks_for_chunk(Chunk* c);

// Store & load player pos, rotation etc.
void db_insert_player_info(Player* p);
void db_get_player_info(Player* p);

// Store & load seed and time of day
void db_insert_map_info();
void db_get_map_info();

void db_close();

#endif