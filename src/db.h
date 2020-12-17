#ifndef DB_H_
#define DB_H_

#include "chunk.h"

void db_init();
void db_insert_block(int chunk_x, int chunk_z, int x, int y, int z, int block);
void db_update_chunk(Chunk* c);
void db_close();

#endif