#pragma once

#include "vec3.h"

class ray
{
public:
	ray() {};
	ray(const vec3& a, const vec3 &b, float ti = 0.0f) { _origin = a; _direction = b; _time = ti; }

	vec3 origin() const { return _origin; }
	vec3 direction() const { return _direction; }
	vec3 point_at_parameter(float t) const { return _origin + _direction * t; }

	float time() const { return _time; }

	vec3 _origin;
	vec3 _direction;

	float _time;
};