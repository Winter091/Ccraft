#ifndef NOISE_GENERATOR_H_
#define NOISE_GENERATOR_H_

#include "fastnoiselite.h"

// Very simple thread-safe rand()
unsigned my_rand(unsigned* prev_value);

typedef struct
{
    fnl_state fnl;
    unsigned rand_value;
}
noise_state;

noise_state* noise_state_create(int cx, int cz);

void noise_set_settings(fnl_state* state, fnl_noise_type noise_type, float freq, 
                        int octaves, float lacunarity, float gain);

void noise_set_return_type(fnl_state* state, fnl_cellular_return_type return_type);

void noise_apply_warp(fnl_state* state, float* x, float* z);

float noise_2d(fnl_state* state, float x, float z);

#endif