#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include "ray.h"

class camera
{
public:
	camera() {}

	// vfov is top of bottom in degree
	camera(const vec3& lookfrom, const vec3& lookat, const vec3& vup, double vfov, double aspect, double aperture, double focus_dist, double t0, double t1)
	{
		time0 = t0;
		time1 = t1;

		// so aperture is diameter...
		lens_radius = aperture / 2;

		double theta = vfov * static_cast<double>(M_PI) / 180.0;
		double half_height = tan(theta / 2);
		double half_width = aspect * half_height;

		origin = lookfrom;
		w = normalize(lookfrom - lookat);
		u = normalize(cross(vup, w));
		v = cross(w, u);
		
		lower_left_corner = origin - half_width * focus_dist *  u - half_height * focus_dist * v - focus_dist * w;
		
		horizontal = 2.0 * half_width * focus_dist * u;
		vertical = 2.0 * half_height * focus_dist * v;		
	}

	ray get_ray(double s, double t)
	{
		vec3 rd = lens_radius * random_in_unit_disk();
		vec3 offset = u * rd.x + v * rd.y;

		double time = time0 + uniform(timeEngine) * (time1 - time0);

		auto dir = lower_left_corner
			+ s * horizontal
			+ t * vertical
			- origin
			- offset;
		return ray(origin + offset, 
			normalize(dir), time);
	}

	vec3 origin;
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 u, v, w;
	double time0, time1;
	double lens_radius;

private:
	vec3 random_in_unit_disk()
	{
		vec3 p;
		do
		{
			p = 2.0 * vec3(uniform(rayEngine), uniform(rayEngine), 0) - vec3(1, 1, 0);
		} while (dot(p, p) >= 1.0);
		return p;
	}

	std::uniform_real_distribution<double> uniform;
	std::minstd_rand rayEngine;
	std::minstd_rand timeEngine;
};