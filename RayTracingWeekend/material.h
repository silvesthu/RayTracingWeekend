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

// a good reference for the math part
// http://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
inline bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted)
{
	vec3 uv = normalize(v);
	// dt = cos(i)
	float dt = dot(uv, n);
	// discriminant = 1 - (ni / nt) ^ 2 * sin(i) ^ 2
	//              = 1 - (sin(t) / sin(i)) ^ 2 * sin(i) ^2
	//				= 1 - sin(t) ^ 2
	//				= cos(t) ^ 2
	float discriminant = 1.0f - ni_over_nt * ni_over_nt * (1 - dt * dt);
	if (discriminant > 0)
	{
		// refracted = ???
		refracted = ni_over_nt * (v - n * dt) - n * sqrt(discriminant);
		return true;
	}
	else
	{
		// total internal reflection
		return false;
	}
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
		attenuation = albedo; // why this is albedo ?
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
		return (dot(scattered.direction(), rec.normal) > 0); // ray from front face ? make no difference in this sample
	}

	vec3 albedo; // metal use albedo ?
	float fuzz;
};

class dielectric : public material
{
public:
	explicit dielectric(float ri) : ref_idx(ri) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const override
	{
		attenuation = vec3(1.0f, 1.0f, 1.0f); // reflect / refract all

		// don't forget to normalize !!!
		vec3 normalizedDirection = normalize(r_in.direction());
				
		vec3 outward_normal;
		float ni_over_nt;
		if (dot(normalizedDirection, rec.normal) > 0)
		{
			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;
		}
		else
		{
			outward_normal = rec.normal;
			ni_over_nt = 1.0f / ref_idx;
		}

		vec3 refracted;
		if (refract(normalizedDirection, outward_normal, ni_over_nt, refracted))
		{
			// refraction
			scattered = ray(rec.p, refracted);
		}
		else
		{
			// total internal reflection
			vec3 reflected = reflect(-normalizedDirection, rec.normal);
			scattered = ray(rec.p, reflected);
		}

		return true;
	}

	float ref_idx;
};