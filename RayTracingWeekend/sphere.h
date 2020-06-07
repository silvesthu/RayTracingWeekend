#pragma once

#include "hittable.h"
#include "material.h"

struct movement_none
{
	vec3 center(const vec3& center0, double time) const
	{
		return center0;
	}

	bool bounding_box(const vec3& center0, double radius, double t0, double t1, aabb& box) const
	{
		box = aabb(center0 - vec3(radius, radius, radius), center0 + vec3(radius, radius, radius));
		return true;
	}
};

struct movement_linear
{
	vec3 center(const vec3& center0, double time) const
	{
		return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
	}

	bool bounding_box(const vec3& center0, double radius, double t0, double t1, aabb& box) const
	{
		auto box0 = aabb(center0 - vec3(radius, radius, radius), center0 + vec3(radius, radius, radius));
		auto box1 = aabb(center1 - vec3(radius, radius, radius), center1 + vec3(radius, radius, radius));
		box = aabb::surrounding(box0, box1);
		return true;
	}

	vec3 center1;
	double time0;
	double time1;
};

template<typename movement_type>
class sphere_base : public hittable
{
public:
	sphere_base() : center(0, 0, 0), radius(0), mat(nullptr) {}
	sphere_base(vec3 cen, double r, std::shared_ptr<material> m) : center(cen), radius(r), mat(m) {}
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override
	{
		vec3 currentCenter = movement.center(center, r.time());
		vec3 oc = r.origin() - currentCenter;
		double a = dot(r.direction(), r.direction());
		double b = dot(oc, r.direction());
		double c = dot(oc, oc) - radius * radius;
		double discriminant = b * b - a * c;
		if (discriminant > 0)
		{
			double temp = (-b - sqrt(discriminant)) / a;
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

	bool bounding_box(double t0, double t1, aabb& box) const override
	{
		return movement.bounding_box(center, radius, t0, t1, box);
	}

	virtual double pdf_value(const vec3& o, const vec3& v) const override
	{
		hit_record rec;
		// ensure hit if direction is right by letting 0.001 < FLT_MAX < +inf
		if (!this->hit(ray(o, v, FLT_MAX), 0.001, std::numeric_limits<double>::infinity(), rec))
			return 0.0;

		double cos_theta_max = sqrt(1 - radius * radius / (center - o).length_squared());
		double solid_angle = 2.0 * M_PI * (1.0 - cos_theta_max);

		return 1.0 / solid_angle;
	}

	virtual vec3 random(const vec3& o) const override
	{
		vec3 direction = center - o;
		double distance_squared = direction.length_squared();
		onb uvw;
		uvw.build_from_w(direction);
		return uvw.local(random_to_sphere(radius, distance_squared));
	}
	
	void set_movement(const movement_type& m)
	{
		movement = m;
	}

	static void get_sphere_uv(const vec3& p, double& u, double& v)
	{
		double phi = atan2(p.z, p.x);
		double theta = asin(p.y);

		u = double(1 - (phi + M_PI) / (2 * M_PI));
		v = double((theta + M_PI / 2) / M_PI);
	}
	
	vec3 center;
	double radius;
	std::shared_ptr<material> mat;
	movement_type movement;
};

typedef sphere_base<movement_none> sphere;
typedef sphere_base<movement_linear> moving_sphere;