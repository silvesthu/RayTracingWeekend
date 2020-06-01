#pragma once

#include "vec3.h"

void get_sphere_uv(const vec3& p, double& u, double& v)
{
	double phi = atan2(p.z, p.x);
	double theta = asin(p.y);
	u = 1 - (phi + M_PI) / (2.0 * M_PI);
	v = (theta + M_PI / 2) / M_PI;
}