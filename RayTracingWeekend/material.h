#pragma once

#include <random>
#include <memory>
#include "onb.h"
#include "hittable.h"
#include "texture.h"
#include "pdf.h"

inline vec3 reflect(const vec3& v, const vec3& n)
{
	return v - 2.0 * dot(v, n) * n;
}

// a good reference for the math part
// http://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
inline bool refract(const vec3& v, const vec3& n, double ni_over_nt, vec3& refracted)
{
	vec3 uv = normalize(v);
	// dt = cos(i)
	double dt = dot(uv, n);
	// discriminant = 1 - (ni / nt) ^ 2 * sin(i) ^ 2
	//              = 1 - (sin(t) / sin(i)) ^ 2 * sin(i) ^2
	//				= 1 - sin(t) ^ 2
	//				= cos(t) ^ 2
	double discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);
	if (discriminant > 0)
	{
		// [Check] refracted = ???
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
inline double schlick(double cosine, double ref_idx)
{
	double r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

struct scatter_record
{
	ray scattered_ray_without_pdf;
	std::shared_ptr<pdf> pdf_ptr;

	vec3 attenuation;
};

class material
{
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const = 0;

	virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const
	{
		return 0.0;
	}

	virtual vec3 emitted(const ray& r_in, const hit_record& rec, double u, double v, const vec3& p) const
	{
		return vec3(0, 0, 0);
	}

	virtual ~material() {}
};

class lambertian : public material
{
public:
	explicit lambertian(std::shared_ptr<texture> a) : albedo(a) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
	{
#if 0 // random_in_unit_sphere - without normalize, ray concentrate on normal direction
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();
		scattered = ray(rec.p, normalize(target - rec.p), r_in.time());
		sampling_pdf = scattering_pdf(r_in, rec, scattered);
#endif // random_in_unit_sphere

#if 0 // random_unit_vector
		vec3 target = rec.p + rec.normal + random_unit_vector();
		scattered = ray(rec.p, normalize(target - rec.p), r_in.time());
		sampling_pdf = scattering_pdf(r_in, rec, scattered);
#endif // random_unit_vector

#if 0 // random_in_hemisphere
		vec3 direction = random_in_hemisphere(rec.normal);
		scattered = ray(rec.p, normalize(direction), r_in.time());
		sampling_pdf = 1.0f / (2.0f * M_PI);
#endif // random_in_hemisphere

#if 0 // onb
		onb uvw;
		uvw.build_from_w(rec.normal);
		vec3 direction = uvw.local(random_cosine_direction());
		scattered = ray(rec.p, normalize(direction), r_in.time());
		sampling_pdf = dot(uvw.w(), scattered.direction()) / M_PI;
#endif // onb

		srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
		srec.pdf_ptr = std::make_shared<cosine_pdf>(rec.normal);

		return true;
	}

	virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const
	{
		double cosine = dot(rec.normal, normalize(scattered.direction()));
		return cosine < 0 ? 0 : cosine / M_PI;
	}

	std::shared_ptr<texture> albedo;
};

class metal : public material
{
public:
	explicit metal(const vec3& a, double f) : albedo(a), fuzz(f) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
	{
		// reflected ray goes to mirror-reflected direction
		vec3 reflected = reflect(normalize(r_in.direction()), rec.normal);
		srec.scattered_ray_without_pdf = ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
		srec.attenuation = albedo;
		srec.pdf_ptr = nullptr;
		return true;
	}

	vec3 albedo;
	double fuzz;
};

class dielectric : public material
{
public:
	explicit dielectric(double ri) : ref_idx(ri) {}
	virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
	{
		srec.attenuation = vec3(1.0, 1.0, 1.0); // reflect / refract all
				
		vec3 outward_normal;
		double ni_over_nt;
		double cosine;
		if (dot(r_in.direction(), rec.normal) > 0)
		{
			// inside -> outside

			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;

			// Why need ref_idx here ?
			// cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();

			// See http://psgraphics.blogspot.com/2016/12/bug-in-my-schlick-code.html
			// cosine -> Theta in Schlick approximation should be the larger "angle"
			// Assume outside is vacuum with lower refractive index,
			// object has refractive index >= 1, thus outside has the larger angle,
			// then we need to calculated the refracted angle for Schlick approximation.

			// Now we need to find cosine of the larger angle.
			// The original implementation in the book scaled the cosine by refractive index,
			// however it is sine that can be scaled by Snell law as described in the link above.
			cosine = dot(r_in.direction(), rec.normal) / r_in.direction().length();
			cosine = sqrt(1 - ref_idx * ref_idx * (1 - cosine * cosine));

			// As why it should be the larger angle,
			// Schlick's approximation is based on a formulation of non-polarized Fresnel.
			// http://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
			// Which is from Peter Shirley's PhD thesis!
			// https://www.cs.utah.edu/~shirley/papers/dissertation.pdf
			// It seems Schlick assume n_i is 1, thus u is cosine of angle on vacuum side.

			// It is possible that object has lower refractive index, even compared with vacuum
			// See https://en.wikipedia.org/wiki/Refractive_index#Refractive_index_below_unity
		}
		else
		{
			// outside -> inside

			outward_normal = rec.normal;
			ni_over_nt = 1.0 / ref_idx;
			cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}

		vec3 reflected = reflect(r_in.direction(), rec.normal);
		vec3 refracted;
		double reflect_prob; // probability
		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
		{
			// refraction or reflection			
			reflect_prob = schlick(cosine, ref_idx);
		}
		else
		{
			// total internal reflection
			srec.scattered_ray_without_pdf = ray(rec.p, reflected, r_in.time());
			reflect_prob = 1.0;
		}

		static std::uniform_real_distribution<double> uniform;
		static std::minstd_rand engine;
		auto rand = uniform(engine);
		if (rand < reflect_prob)
		{
			srec.scattered_ray_without_pdf = ray(rec.p, reflected, r_in.time());
		} 
		else
		{
			srec.scattered_ray_without_pdf = ray(rec.p, refracted, r_in.time());
		}

		return true;
	}

	double ref_idx;
};

class diffuse_light : public material
{
public:
	diffuse_light(std::shared_ptr<texture> a) : emit(a) {}

	bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
	{
		// as light source, no reflection
		return false;
	}

	vec3 emitted(const ray& r_in, const hit_record& rec, double u, double v, const vec3& p) const override
	{
		if (dot(rec.normal, r_in.direction()) > 0)
			return emit->value(u, v, p);
		else
			return vec3(0, 0, 0);
	}

	std::shared_ptr<texture> emit;
};

// from author's comment (detail not in book)
// http://in1weekend.blogspot.jp/2016/01/ray-tracing-second-weekend.html?showComment=1461950451498#c5689943635697942364
// isotropic here means scatter light in every direction equally
class isotropic : public material
{
public:
	isotropic(std::shared_ptr<texture> t) : albedo(t) {}

	bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
	{
		srec.scattered_ray_without_pdf = ray(rec.p, random_in_unit_sphere(), r_in.time());
		srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
		return true;
	}

	std::shared_ptr<texture> albedo;
};