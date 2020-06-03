#pragma once

#include "vec3.h"

class ray
{
public:
	ray() {};
	ray(const vec3& a, const vec3 &b, double ti) { _origin = a; _direction = b; _time = ti; }
	vec3 origin() const { return _origin; }	
	vec3 direction() const { return _direction; } // direction is not normalized !!!
	vec3 point_at_parameter(double t) const { return _origin + _direction * t; }

	double time() const { return _time; }

private:
	vec3 _origin;
	vec3 _direction;

	double _time;
};