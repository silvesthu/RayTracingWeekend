#pragma once

#include "vec3.h"

class ray
{
public:
	ray() {};
	ray(const vec3& a, const vec3 &b) { _origin = a; _direction = b; }

	vec3 origin() const { return _origin; }
	vec3 direction() const { return _direction; }
	vec3 point_at_parameter(float t) const { return _origin + _direction * t; }

	vec3 _origin;
	vec3 _direction;
};