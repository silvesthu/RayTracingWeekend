#pragma once

#include "hitable.h"
#include "material.h"

struct trivial_strategy
{
	vec3 center(const vec3& c, float time) const
	{
		return c;
	}
};

struct moving_strategy
{
	vec3 center(const vec3& c, float time) const
	{
		return c + ((time - time0) / (time1 - time0)) * (center1 - c);
	}

	vec3 center1;
	float time0;
	float time1;
};

template<typename Strategy>
class sphere_base : public hitable
{
public:
	sphere_base() : center(0, 0, 0), radius(0), mat(nullptr) {}
	sphere_base(vec3 cen, float r, std::unique_ptr<material> m) : center(cen), radius(r), mat(std::move(m)) {}
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override
	{
		vec3 currentCenter = strategy.center(center, r.time());
		vec3 oc = r.origin() - currentCenter;
		float a = dot(r.direction(), r.direction());
		float b = dot(oc, r.direction());
		float c = dot(oc, oc) - radius * radius;
		float discriminant = b * b - a * c;
		if (discriminant > 0)
		{
			float temp = (-b - sqrt(discriminant)) / a;
			if (temp < t_max && temp > t_min)
			{
				// hit on near point
				rec.t = temp;
				rec.p = r.point_at_parameter(rec.t);
				rec.normal = (rec.p - currentCenter) / radius;
				rec.mat_ptr = mat.get();
				return true;
			}
			temp = (-b + sqrt(discriminant)) / a;
			if (temp < t_max && temp > t_min)
			{
				// hit on far point
				rec.t = temp;
				rec.p = r.point_at_parameter(rec.t);
				rec.normal = (rec.p - currentCenter) / radius;
				rec.mat_ptr = mat.get();
				return true;
			}
		}

		return false;
	}
	
	void update_strategy(const Strategy& s)
	{
		strategy = s;
	}

	vec3 center;
	float radius;
	std::unique_ptr<material> mat;
	Strategy strategy;
};

typedef sphere_base<trivial_strategy> sphere;
typedef sphere_base<moving_strategy> moving_sphere;