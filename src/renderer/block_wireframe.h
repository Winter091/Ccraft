#ifndef BLOCK_WIREFRAME_H_
#define BLOCK_WIREFRAME_H_

#include <cglm/cglm.h>

void renderer_block_wireframe_init();

void renderer_block_wireframe_render(mat4 cam_vp_matrix, vec3 block_aabb[2]);

void renderer_block_wireframe_free();

#endif
