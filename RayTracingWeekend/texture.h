#pragma once

#include <algorithm>
#include <memory>

#include "vec3.h"
#include "ray.h"

class texture
{
public:
	virtual vec3 value(float u, float v, const vec3& p) const = 0;
};

class constant_texture : public texture
{
public:
	constant_texture() {}
	constant_texture(vec3 c) : color(c) {}
	virtual vec3 value(float u, float v, const vec3& p) const
	{
		return color;
	}

	vec3 color;
};

class checker_texture : public texture
{
public:
	checker_texture() {}
	checker_texture(std::shared_ptr<texture>& t0, std::shared_ptr<texture>& t1) : even(t0), odd(t1) {}

	virtual vec3 value(float u, float v, const vec3& p) const
	{
		float sines = std::sin(10.0f * p.x) * std::sin(10.0f * p.y) * std::sin(10.0f * p.z);
		if (sines < 0)
		{
			return odd->value(u, v, p);
		}
		else
		{
			return even->value(u, v, p);
		}
	}

	std::shared_ptr<texture> odd;
	std::shared_ptr<texture> even;
};