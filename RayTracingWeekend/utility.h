#pragma once

#include <random>
#include "vec3.h"

void get_sphere_uv(const vec3& p, double& u, double& v)
{
	double phi = atan2(p.z, p.x);
	double theta = asin(p.y);
	u = 1 - (phi + M_PI) / (2.0 * M_PI);
	v = (theta + M_PI / 2) / M_PI;
}

double random_double(double a = 0.0, double b = 1.0)
{
	static std::uniform_real_distribution<double> uniform;
	static std::minstd_rand engine;

	return a + (b - a) * uniform(engine);
}