#pragma once

#include <random>
#include "hitable.h"

inline vec3 random_in_unit_sphere()
{
	static std::uniform_real<float> uniform;
	static std::minstd_rand engine;

	vec3 p = { 0, 0, 0 };
	do {
		auto random_vector = vec3(uniform(engine), uniform(engine), uniform(engine));
		p = 2.0f * random_vector - vec3(1, 1, 1); // -1 ~ 1 box
	} while (dot(p, p) >= 1.0f); // unit sphere
	return p;
}

inline vec3 reflect(const vec3& v, const vec3& n)
{
	return v - 2.0f * dot(v, n) * n;
}

class material
{
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
	virtual ~material() {}
};

class lambertian : public material
{
public:
	explicit lambertian(const vec3& a) : albedo(a) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const override
	{
		// reflected ray goes to random direction
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();
		scattered = ray(rec.p, target - rec.p);
		attenuation = albedo; // ?
		return true;
	}

	vec3 albedo;
};

class metal : public material
{
public:
	explicit metal(const vec3& a, float f) : albedo(a), fuzz(f) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const override
	{
		// reflected ray goes to mirror-reflected direction
		vec3 reflected = reflect(normalize(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0); // ray from front face ? make no difference in this sample now
	}

	vec3 albedo; // metal use albedo ?
	float fuzz;
};