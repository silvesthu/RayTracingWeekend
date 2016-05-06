#pragma once

#include <algorithm>

#include "vec3.h"
#include "ray.h"

class aabb
{
public:
	aabb() {};
	aabb(const vec3& a, const vec3 &b) { _min = a; _max = b; }

	vec3 min() const { return _min; }
	vec3 max() const { return _max; }

	bool hit(const ray& r, float tmin, float tmax) const
	{
		// "slab" method
		for (int axis = 0; axis < 3; axis++)
		{
			float invD = 1.0f / r.direction()[axis];

			// calculate t to let ray reach two side of aabb
			float t0 = (min()[axis] - r.origin()[axis]) * invD;
			float t1 = (max()[axis] - r.origin()[axis]) * invD;

			if (invD < 0.0f)
			{
				// flip when ray is towards minus
				std::swap(t0, t1);
			}

			// no need to use std::fmax since boundary condition is already considered
			tmin = std::max(t0, tmin);
			tmax = std::min(t1, tmax);

			// check if there is overlap between slabs (formed by intersections)
			// slabs are along ray direction (1D range), so compare t is sufficient
			if (tmax <= tmin)
			{
				return false;
			}
		}

		return true;
	}
	
	vec3 _min;
	vec3 _max;
};