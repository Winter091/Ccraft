#include "db.h"

#include "sqlite3.h"
#include "config.h"
#include "stdio.h"

sqlite3* db;

static sqlite3_stmt* db_compile_statement(const char* statement)
{
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, statement, -1, &stmt, NULL);
    return stmt;
}

static void db_compile_run_statement(char* statement)
{
    sqlite3_stmt* stmt = db_compile_statement(statement);
    if (!stmt)
    {
        fprintf(stderr, "DB error during statement compiling:\n");
        fprintf(stderr, "Statement: %s\n", statement);
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void db_init()
{
    sqlite3_open(CURRENT_MAP, &db);

    // create blocks table if it does not exist yet
    db_compile_run_statement(
        "CREATE TABLE IF NOT EXISTS blocks("
            "chunk_x INTEGER NOT NULL, "
            "chunk_z INTEGER NOT NULL, "
            "x INTEGER NOT NULL, "
            "y INTEGER NOT NULL, "
            "z INTEGER NOT NULL, "
            "block INTEGER NOT NULL, "
            "PRIMARY KEY(chunk_x, chunk_z, x, y, z)"
        ")"
    );

    // optimizations to make db run much faster
    db_compile_run_statement("PRAGMA synchronous = off");
    db_compile_run_statement("PRAGMA temp_store = memory");
    db_compile_run_statement("PRAGMA locking_mode = exclusive");
 
}

void db_insert_block(int chunk_x, int chunk_z, int x, int y, int z, int block)
{
    static const char* query = 
        "INSERT OR REPLACE INTO "
        "blocks (chunk_x, chunk_z, x, y, z, block) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    static sqlite3_stmt* stmt = NULL;
    if (stmt == NULL)
    {
        stmt = db_compile_statement(query);
    }

    sqlite3_reset(stmt);

    // pass variables
    sqlite3_bind_int(stmt, 1, chunk_x);
    sqlite3_bind_int(stmt, 2, chunk_z);
    sqlite3_bind_int(stmt, 3, x);
    sqlite3_bind_int(stmt, 4, y);
    sqlite3_bind_int(stmt, 5, z);
    sqlite3_bind_int(stmt, 6, block);

    // execute command
    sqlite3_step(stmt);
}

void db_update_chunk(Chunk* c)
{
    static const char* query = 
        "SELECT x, y, z, block "
        "FROM blocks "
        "WHERE chunk_x = ? AND chunk_z = ?";
    
    static sqlite3_stmt* stmt = NULL;
    if (stmt == NULL)
    {
        stmt = db_compile_statement(query);
    }

    sqlite3_reset(stmt);

    sqlite3_bind_int(stmt, 1, c->x);
    sqlite3_bind_int(stmt, 2, c->z);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int x = sqlite3_column_int(stmt, 0);
        int y = sqlite3_column_int(stmt, 1);
        int z = sqlite3_column_int(stmt, 2);
        int block = sqlite3_column_int(stmt, 3);

        if (x < CHUNK_WIDTH && y < CHUNK_HEIGHT && z < CHUNK_WIDTH)
            c->blocks[XYZ(x, y, z)] = block;
    }
}

void db_close()
{
    sqlite3_close(db);
}