#pragma once

#include "hitable.h"
#include "material.h"

class sphere : public hitable
{
public:
	sphere() : center(0, 0, 0), radius(0), mat(nullptr) {}
	sphere(vec3 cen, float r, std::unique_ptr<material> m) : center(cen), radius(r), mat(std::move(m)) {}
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override
	{
		vec3 oc = r.origin() - center;
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
				rec.normal = (rec.p - center) / radius;
				rec.mat_ptr = mat.get();
				return true;
			}
			temp = (-b + sqrt(discriminant)) / a;
			if (temp < t_max && temp > t_min)
			{
				// hit on far point
				rec.t = temp;
				rec.p = r.point_at_parameter(rec.t);
				rec.normal = (rec.p - center) / radius;
				rec.mat_ptr = mat.get();
				return true;
			}
		}

		return false;
	}
	vec3 center;
	float radius;
	std::unique_ptr<material> mat;
};