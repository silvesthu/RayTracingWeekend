#pragma once

#include "hitable.h"
#include "material.h"

struct movement_none
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

struct movement_linear
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

template<typename movement_type>
class sphere_base : public hitable
{
public:
	sphere_base() : center(0, 0, 0), radius(0), mat(nullptr) {}
	sphere_base(vec3 cen, float r, std::shared_ptr<material> m) : center(cen), radius(r), mat(m) {}
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override
	{
		vec3 currentCenter = movement.center(center, r.time());
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
				get_sphere_uv(rec.normal, rec.u, rec.v);
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
				get_sphere_uv(rec.normal, rec.u, rec.v);
				rec.mat_ptr = mat.get();
				return true;
			}
		}

		return false;
	}

	bool bounding_box(float t0, float t1, aabb& box) const override
	{
		return movement.bounding_box(center, radius, t0, t1, box);
	}
	
	void set_movement(const movement_type& m)
	{
		movement = m;
	}

	static void get_sphere_uv(const vec3& p, float& u, float& v)
	{
		float phi = atan2(p.z, p.x);
		float theta = asin(p.y);

		u = float(1 - (phi + M_PI) / (2 * M_PI));
		v = float((theta + M_PI / 2) / M_PI);
	}
	
	vec3 center;
	float radius;
	std::shared_ptr<material> mat;
	movement_type movement;
};

typedef sphere_base<movement_none> sphere;
typedef sphere_base<movement_linear> moving_sphere;