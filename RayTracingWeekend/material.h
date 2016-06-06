#pragma once

#include <random>
#include <memory>
#include "hitable.h"
#include "texture.h"

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
		// don't forget to use normalized uv !!!
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
		return true;
	}
	else
	{
		// total internal reflection
		return false;
	}
}

// the schlick approximation for fresnel factor (specular reflection coefficient)
// R(theta) = R0 + (1 - R0)(1- cos(theta)) ^ 5
// R0 = ((n1 - n2) / (n1 + n2)) ^ 2
inline float schlick(float cosine, float ref_idx)
{
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
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
	explicit lambertian(std::unique_ptr<texture>& a) : albedo(std::move(a)) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const override
	{
		// reflected ray goes to random direction
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();
		scattered = ray(rec.p, target - rec.p);
		attenuation = albedo->value(0, 0, rec.p);
		return true;
	}

	std::unique_ptr<texture> albedo;
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
				
		vec3 outward_normal;
		float ni_over_nt;
		float cosine;
		if (dot(r_in.direction(), rec.normal) > 0)
		{
			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;

			// why need ref_idx here ?
			cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}
		else
		{
			outward_normal = rec.normal;
			ni_over_nt = 1.0f / ref_idx;
			cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}

		vec3 reflected = reflect(r_in.direction(), rec.normal);
		vec3 refracted;
		float reflect_prob; // probability
		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
		{
			// refraction or reflection			
			reflect_prob = schlick(cosine, ref_idx);
		}
		else
		{
			// total internal reflection
			scattered = ray(rec.p, reflected);
			reflect_prob = 1.0f;
		}

		static std::uniform_real<float> uniform;
		static std::minstd_rand engine;

		auto rand = uniform(engine);
		if (rand < reflect_prob)
		{
			scattered = ray(rec.p, reflected);
		} 
		else
		{
			scattered = ray(rec.p, refracted);
		}

		return true;
	}

	float ref_idx;
};