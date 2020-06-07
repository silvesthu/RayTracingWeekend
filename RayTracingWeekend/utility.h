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

int random_int(int a, int b)
{
	return a + std::min(b - a, (int)((b - a + 1) * random_double()));
}

inline vec3 random_in_unit_sphere()
{
	vec3 p = { 0, 0, 0 };
	do {
		vec3 random_vector(random_double(), random_double(), random_double());
		p = 2.0 * random_vector - vec3(1, 1, 1); // -1 ~ 1 box
	} while (dot(p, p) >= 1.0); // unit sphere
	return p;
}

inline vec3 random_unit_vector()
{
	auto a = random_double() * 2.0 * M_PI;
	auto z = random_double() * 2.0 - 1.0;
	auto r = sqrt(1 - z * z);
	return vec3(r * cos(a), r * sin(a), z);
}

inline vec3 random_in_hemisphere(const vec3& normal)
{
	vec3 in_unit_sphere = random_in_unit_sphere();
	if (dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
		return in_unit_sphere;
	else
		return -in_unit_sphere;
}

inline vec3 random_cosine_direction()
{
	// see book3.chapter7.2

	double r1 = random_double();
	double r2 = random_double();
	double z = sqrt(1 - r2);

	double phi = 2 * M_PI * r1;
	double x = cos(phi) * sqrt(r2);
	double y = sin(phi) * sqrt(r2);
	
	return vec3(x, y, z);
}

inline vec3 random_to_sphere(double radius, double distance_squared) 
{
	// see book3.chapter12.3

	double r1 = random_double();
	double r2 = random_double();
	double z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

	double phi = 2 * M_PI * r1;
	double x = cos(phi) * sqrt(1 - z * z);
	double y = sin(phi) * sqrt(1 - z * z);

	return vec3(x, y, z);
}