#pragma once

#include "hitable.h"
#include "material.h"

struct trivial_strategy
{
	vec3 center(const vec3& center0, float time) const
	{
		return center0;
	}

	bool bounding_box(const vec3& center0, float radius, float t0, float t1, aabb& box) const
	{
		box = aabb(center0 - vec3(radius, radius, radius), center0 + vec3(radius, radius, radius));
		return true;
	}
};

struct moving_strategy
{
	vec3 center(const vec3& center0, float time) const
	{
		return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
	}

	bool bounding_box(const vec3& center0, float radius, float t0, float t1, aabb& box) const
	{
		auto box0 = aabb(center0 - vec3(radius, radius, radius), center0 + vec3(radius, radius, radius));
		auto box1 = aabb(center1 - vec3(radius, radius, radius), center1 + vec3(radius, radius, radius));
		box = aabb::surrounding(box0, box1);
		return true;
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

	virtual bool bounding_box(float t0, float t1, aabb& box) const
	{
		return strategy.bounding_box(center, radius, t0, t1, box);
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