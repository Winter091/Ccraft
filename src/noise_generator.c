#include "stdlib.h"

#include "noise_generator.h"
#include "map.h"

unsigned my_rand(unsigned* prev_value)
{
    *prev_value = *prev_value * 1103515245 + 12345;
    return *prev_value;
}

noise_state* noise_state_create(int cx, int cz)
{
    noise_state* state = malloc(sizeof(noise_state));

    state->fnl = fnlCreateState();
    state->fnl.fractal_type = FNL_FRACTAL_FBM;
    state->fnl.cellular_distance_func = FNL_CELLULAR_DISTANCE_EUCLIDEAN;
    state->fnl.domain_warp_amp = 100.0f;
    state->fnl.domain_warp_type = FNL_DOMAIN_WARP_OPENSIMPLEX2;
    state->fnl.seed = map_get_seed();

    state->rand_value = (cx << 16) ^ cz;

    return state;
}

void noise_set_settings(fnl_state* state, fnl_noise_type noise_type, float freq, 
                        int octaves, float lacunarity, float gain)
{
    state->noise_type = noise_type;
    state->frequency = freq;
    state->octaves = octaves;
    state->lacunarity = lacunarity;
    state->gain = gain;
}

// [0.0, 1.0]
float noise_2d(fnl_state* state, float x, float z)
{
    return (fnlGetNoise2D(state, x, z) + 1.0f) / 2.0f;
}