#include <db.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <tinycthread.h>
#include <sqlite3.h>

#include <config.h>
#include <map/map.h>

static sqlite3* db;
static int s_has_player_info;
static int s_has_map_info;

static sqlite3_stmt* db_compile_statement(const char* statement)
{
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, statement, -1, &stmt, NULL);
    if (!stmt)
    {
        fprintf(stderr, "DB error during statement compiling:\n");
        fprintf(stderr, "Statement: %s\n", statement);
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
    }
    return stmt;
}

static void db_compile_run_statement(const char* statement)
{
    sqlite3_stmt* stmt = db_compile_statement(statement);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

static int db_is_table_empty(const char* table)
{
    char query[512];
    sprintf(query, "SELECT COUNT(*) FROM %s", table);
    
    sqlite3_stmt* stmt = db_compile_statement(query);

    sqlite3_bind_text(stmt, 1, table, -1, SQLITE_STATIC);
    sqlite3_step(stmt);

    int row_count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return row_count == 0;
}

static void db_create_tables()
{
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

    db_compile_run_statement(
        "CREATE TABLE IF NOT EXISTS map_info("
            "seed         INTEGER NOT NULL, "
            "curr_time    REAL NOT NULL, "
            "chunk_width  INTEGER NOT NULL, "
            "chunk_height INTEGER NOT NULL"
        ")"
    );

    db_compile_run_statement(
        "CREATE TABLE IF NOT EXISTS player_info("
            "pos_x REAL NOT NULL, "
            "pos_y REAL NOT NULL, "
            "pos_z REAL NOT NULL, "
            "pitch REAL NOT NULL, "
            "yaw REAL NOT NULL, "
            "build_block INTEGER NOT NULL"
        ")"
    );

    if (!db_is_table_empty("map_info"))
        s_has_map_info = 1;

    if (!db_is_table_empty("player_info"))
        s_has_player_info = 1;
}

void db_init(const char* db_path)
{
    int result = sqlite3_open(db_path, &db);
    if (result != SQLITE_OK)
    {
        fprintf(stderr, "Error trying to open database %s:\n", db_path);
        const char* msg = sqlite3_errmsg(db);
        fprintf(stderr, msg);
        exit(EXIT_FAILURE);
    }

    if (!sqlite3_threadsafe())
    {
        fprintf(stderr, "sqlite3 connection has to be threadsafe, but is not!\n");
        exit(EXIT_FAILURE);
    }

    db_create_tables();

    // optimizations to make db run much faster
    db_compile_run_statement("PRAGMA synchronous = off");
    db_compile_run_statement("PRAGMA temp_store = memory");
    db_compile_run_statement("PRAGMA locking_mode = exclusive");
}

void db_insert_block(int chunk_x, int chunk_z, int x, int y, int z, int block)
{
    static sqlite3_stmt* stmt = NULL;
    if (stmt == NULL)
    {
        stmt = db_compile_statement(
            "INSERT OR REPLACE INTO "
            "blocks (chunk_x, chunk_z, x, y, z, block) "
            "VALUES (?, ?, ?, ?, ?, ?)"
        );
    }

    sqlite3_reset(stmt);

    sqlite3_bind_int(stmt, 1, chunk_x);
    sqlite3_bind_int(stmt, 2, chunk_z);
    sqlite3_bind_int(stmt, 3, x);
    sqlite3_bind_int(stmt, 4, y);
    sqlite3_bind_int(stmt, 5, z);
    sqlite3_bind_int(stmt, 6, block);

    sqlite3_step(stmt);
}

void db_get_blocks_for_chunk(Chunk* c)
{
    static sqlite3_stmt* stmt = NULL;
    if (stmt == NULL)
    {
        stmt = db_compile_statement(
            "SELECT x, y, z, block "
            "FROM blocks "
            "WHERE chunk_x = ? AND chunk_z = ?"
        );
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

        c->blocks[XYZ(x, y, z)] = block;
    }
}

void db_save_player_info(Player* p)
{
    sqlite3_stmt* stmt;
    if (s_has_player_info)
    {
        stmt = db_compile_statement(
            "UPDATE player_info " 
            "SET pos_x = ?, pos_y = ?, pos_z = ?, " 
            "pitch = ?, yaw = ?, "
            "build_block = ?"
        );
    }
    else
    {
        stmt = db_compile_statement(
            "INSERT INTO player_info (pos_x, pos_y, pos_z, "
            "pitch, yaw, build_block) "
            "VALUES (?, ?, ?, ?, ?, ?)"
        );
    }

    sqlite3_bind_double(stmt, 1, p->pos[0]);
    sqlite3_bind_double(stmt, 2, p->pos[1]);
    sqlite3_bind_double(stmt, 3, p->pos[2]);
    sqlite3_bind_double(stmt, 4, p->pitch);
    sqlite3_bind_double(stmt, 5, p->yaw);
    sqlite3_bind_int(stmt, 6, p->build_block);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    s_has_player_info = 1;
}

void db_load_player_info(Player* p)
{
    assert(s_has_player_info);
    
    sqlite3_stmt* stmt = db_compile_statement(
        "SELECT pos_x, pos_y, pos_z, pitch, yaw, build_block "
        "FROM player_info "
    );

    sqlite3_reset(stmt);
    sqlite3_step(stmt);

    p->pos[0] = sqlite3_column_double(stmt, 0);
    p->pos[1] = sqlite3_column_double(stmt, 1);
    p->pos[2] = sqlite3_column_double(stmt, 2);
    p->pitch  = sqlite3_column_double(stmt, 3);
    p->yaw    = sqlite3_column_double(stmt, 4);
    p->build_block = sqlite3_column_int(stmt, 5);

    sqlite3_finalize(stmt);
}

int db_has_player_info()
{
    return s_has_player_info;
}

void db_save_map_info()
{
    sqlite3_stmt* stmt;
    if (s_has_map_info)
    {
        stmt = db_compile_statement(
            "UPDATE map_info SET curr_time = ?"
        );
        sqlite3_bind_double(stmt, 1, map_get_time());
    }
    else
    {
        stmt = db_compile_statement(
            "INSERT INTO map_info (seed, curr_time, chunk_width, chunk_height) "
            "VALUES (?, ?, ?, ?)"
        );
        sqlite3_bind_int(stmt, 1, map_get_seed());
        sqlite3_bind_double(stmt, 2, map_get_time());
        sqlite3_bind_int(stmt, 3, CHUNK_WIDTH);
        sqlite3_bind_int(stmt, 4, CHUNK_HEIGHT);
    }

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    s_has_map_info = 1;
}

void db_load_map_info()
{
    assert(s_has_map_info);
    
    sqlite3_stmt* stmt = db_compile_statement(
        "SELECT seed, curr_time, chunk_width, chunk_height "
        "FROM map_info"
    );

    sqlite3_reset(stmt);
    sqlite3_step(stmt);

    int seed = sqlite3_column_int(stmt, 0);
    double curr_time = sqlite3_column_double(stmt, 1);
    int width = sqlite3_column_int(stmt, 2);
    int height = sqlite3_column_int(stmt, 3);

    if (width != CHUNK_WIDTH || height != CHUNK_HEIGHT)
    {
        fprintf(
            stderr, 
            "The map \"%s\" was created for use with CHUNK_WIDTH = %d, "
            "CHUNK_HEIGHT = %d, and therefore incompatible with "
            "current width and height (%d, %d). Use another map or "
            "chunk dimensions!\n", 
            MAP_NAME, width, height, CHUNK_WIDTH, CHUNK_HEIGHT
        );
        exit(EXIT_FAILURE);
    }

    map_set_seed(seed);
    map_set_time(curr_time);
    sqlite3_finalize(stmt);
}

int db_has_map_info()
{
    return s_has_map_info;
}

void db_free()
{
    sqlite3_close(db);
}