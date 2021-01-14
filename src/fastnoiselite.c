#define FNL_IMPL
#include "fastnoiselite.h"

static fnl_state noise_state;

void noise_init()
{
    noise_state = fnlCreateState();
    noise_state.fractal_type = FNL_FRACTAL_FBM;
    noise_state.cellular_distance_func = FNL_CELLULAR_DISTANCE_EUCLIDEAN;
    noise_state.domain_warp_amp = 100.0f;
    noise_state.domain_warp_type = FNL_DOMAIN_WARP_OPENSIMPLEX2;
}

void noise_set_seed(int new_seed)
{
    noise_state.seed = new_seed;
}

int noise_get_seed()
{
    return noise_state.seed;
}

void noise_set_settings(fnl_noise_type noise_type, float freq, int octaves, float lacunarity, float gain)
{
    noise_state.noise_type = noise_type;
    noise_state.frequency = freq;
    noise_state.octaves = octaves;
    noise_state.lacunarity = lacunarity;
    noise_state.gain = gain;
}

void noise_set_return_type(fnl_cellular_return_type return_type)
{
    noise_state.cellular_return_type = return_type;
}

void noise_apply_warp(float* x, float* z)
{
    fnlDomainWarp2D(&noise_state, x, z);
}

float noise_2d(float x, float z)
{
    return (fnlGetNoise2D(&noise_state, x, z) + 1.0f) / 2.0f;
}