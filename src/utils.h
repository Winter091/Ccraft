#ifndef UTILS_H_
#define UTILS_H_

#include "stdlib.h"
#include "string.h"

#include "cglm/cglm.h"
#include "glad/glad.h"
#include "config.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
    #define PLATFORM_WINDOWS
    #include "Windows.h"
#else
    #define PLATFORM_POSIX
    #include "unistd.h"
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Vertex layout for storing block data in GPU
typedef struct
{
    float pos[3];
    float tex_coord[2];
    float ao;
    unsigned char tile;
    unsigned char normal;
}
Vertex;

GLuint opengl_create_vao();

GLuint opengl_create_vbo(const void* vertices, size_t buf_size);

GLuint opengl_create_vbo_cube();

GLuint opengl_create_vbo_quad();

GLuint opengl_create_fbo();

// Get next smallest value that is divisible by 8:
// 5 -> 0, 9 -> 8, -1 -> -8, -8 -> -8, ...
static inline int floor8(int a)
{
    if (a >= 0)
        return (int)(a / 8) * 8;
    else
        return ((int)((a + 1) / 8) - 1) * 8;
}

static inline int thread_hardware_concurrency()
{
#if defined(PLATFORM_WINDOWS)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif defined(PLATFORM_POSIX)
    return sysconf(_SC_NPROCESSORS_ONLN);
#else
    // Why are we here? Just to suffer?
    return 1;
#endif
}

static inline void my_glm_vec3_set(vec3 vec, float f0, float f1, float f2)
{
    vec[0] = f0;
    vec[1] = f1;
    vec[2] = f2;
}

static inline void my_glm_ivec3_set(ivec3 vec, int i0, int i1, int i2)
{
    vec[0] = i0;
    vec[1] = i1;
    vec[2] = i2;
}

static inline int chunked_block(int world_block_coord)
{
    if (world_block_coord >= 0) 
        return world_block_coord / CHUNK_WIDTH;
    return (world_block_coord + 1) / CHUNK_WIDTH - 1;
}

static inline int to_chunk_coord(int world_block_coord)
{
    int block = world_block_coord % CHUNK_WIDTH;
    if (block < 0) block += CHUNK_WIDTH;
    return block;
}

static inline int chunked_cam(float cam_coord)
{
    return (int)(cam_coord / CHUNK_SIZE);
}

static inline float blocked(float coord)
{
    return coord / BLOCK_SIZE;
}

static inline int chunk_player_dist2(int cx, int cz, int px, int pz)
{
    return (cx - px) * (cx - px) + (cz - pz) * (cz - pz);
}

static inline float block_player_dist2(int bx, int by, int bz, float px, float py, float pz)
{
    return (bx - px) * (bx - px) + (by - py) * (by - py) + (bz - pz) * (bz - pz);
}

static inline void opengl_vbo_layout(
    GLuint index, GLint size, GLenum type, GLboolean normalized,
    GLsizei stride, size_t offset
)
{
    if (type >= GL_BYTE && type <= GL_UNSIGNED_INT)
        glVertexAttribIPointer(index, size, type, stride, (const void*)offset);
    else
        glVertexAttribPointer(index, size, type, normalized, stride, (const void*)offset);

    glEnableVertexAttribArray(index);
}

static inline const char* my_strdup(const char* src)
{
    const size_t buf_size = strlen(src) + 1;
    const char* new_str = malloc(buf_size);
    memcpy((void*)new_str, src, buf_size);
    return new_str;
}

#endif