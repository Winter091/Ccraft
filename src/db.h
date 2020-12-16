#ifndef DB_H_
#define DB_H_

void db_init();
void db_set_block(int chunk_x, int chunk_z, int x, int y, int z, int block);
void db_close();

#endif