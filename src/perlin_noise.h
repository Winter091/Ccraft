#ifndef PERLIN_NOISE_H_
#define PERLIN_NOISE_H_

#include "stdint.h"

uint16_t* perlin2d_get_world_seed();

float perlin2d(float x, float y, int octaves, float persistence, float lacunarity, float amplitude);

#endif