#ifndef DB_H_
#define DB_H_

#include "chunk.h"
#include "player.h"

void db_init();

void db_insert_block(int chunk_x, int chunk_z, int x, int y, int z, int block);
void db_get_blocks_for_chunk(Chunk* c);

void db_insert_player_info(Player* p);
void db_get_player_info(Player* p);

void db_insert_map_info();
void db_get_map_info();

void db_close();

#endif