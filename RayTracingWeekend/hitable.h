#pragma once

#include "vec3.h"
#include "ray.h"

class material;

struct hit_record
{
	hit_record()
	{
		mat_ptr = nullptr;
		t = 0;
	}

	float t;
	vec3 p;
	vec3 normal; // should filled with normalized normal
	material *mat_ptr;
};

class hitable
{
public:
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
	virtual ~hitable() {}
};
