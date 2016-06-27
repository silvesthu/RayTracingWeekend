#pragma once

#include <algorithm>
#include <memory>

#include "vec3.h"
#include "ray.h"
#include "noise.h"

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

class noise_texture : public texture
{
public:
	noise_texture() : scale(5.f) {}
	noise_texture(float sc) : scale(sc) {}
	virtual vec3 value(float u, float v, const vec3& p) const
	{
		// static std::uniform_real<float> uniform;
		// static std::minstd_rand engine;
		// return vec3(1, 1, 1) * uniform(engine);

		// return vec3(1, 1, 1) * (noise.noise(scale * p) + 1.0f) * 0.5f;

		// return vec3(1, 1, 1) * noise.turb(scale * p);

		return vec3(1, 1, 1) * 0.5f * (1 + sin(scale * p.z + 10 * noise.turb(p)));
	}
	perlin noise;
	float scale;
};