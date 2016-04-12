#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include "ray.h"

class camera
{
public:
	// vfov is top of bottom in degree
	camera(vec3& lookfrom, vec3& lookat, vec3& vup, float vfov, float aspect, float aperture, float focus_dist)
	{
		// so aperture is diameter...
		lens_radius = aperture / 2;

		float theta = vfov * static_cast<float>(M_PI) / 180.0f;
		float half_height = tan(theta / 2);
		float half_width = aspect * half_height;

		origin = lookfrom;
		vec3 w = normalize(lookfrom - lookat);
		vec3 u = normalize(cross(vup, w));
		vec3 v = cross(w, u);
		
		lower_left_corner = origin - half_width * focus_dist *  u - half_height * focus_dist * v - focus_dist * w;
		
		horizontal = 2.0f * half_width * focus_dist * u;
		vertical = 2.0f * half_height * focus_dist * v;		
	}

	ray get_ray(float u, float v)
	{
		vec3 rd = lens_radius * random_in_unit_disk();
		vec3 offset = u * rd.x + v * rd.y;

		offset = vec3(0, 0, 0); // test : no offset

		return ray(origin + offset, 
			lower_left_corner 
			+ u * horizontal 
			+ v * vertical 
			- origin 
			- offset);
	}

	vec3 origin;
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;

	float lens_radius;

private:
	vec3 random_in_unit_disk()
	{
		vec3 p;
		do
		{
			p = 2.0f * vec3(uniform(engine), uniform(engine), 0) - vec3(1, 1, 0);
		} while (dot(p, p) >= 1.0f);
		return p;
	}

	std::uniform_real<float> uniform;
	std::minstd_rand engine;
};