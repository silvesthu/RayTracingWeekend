#include "noise.h"

float perlin::ranfloat[perlin::SIZE];
vec3 perlin::ranvec[perlin::SIZE];
int perlin::perm_x[perlin::SIZE];
int perlin::perm_y[perlin::SIZE];
int perlin::perm_z[perlin::SIZE];

bool perlin::initialized = false;
