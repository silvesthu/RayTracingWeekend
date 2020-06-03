#pragma once

#define _USE_MATH_DEFINES

#include <iostream>
#include "math.h"

#include "vec3.h"
#include "ray.h"
#include "aabb.h"
#include "texture.h"
#include "utility.h"

class material;

struct hit_record
{
	hit_record()
	{
		mat_ptr = nullptr;
		t = 0;
	}
	double t;
	vec3 p;
	vec3 normal; // should filled with normalized normal
	double u;
	double v;
	material *mat_ptr;
};

class hittable
{
public:
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
	virtual bool bounding_box(double t0, double t1, aabb& box) const = 0;
	virtual double pdf_value(const vec3& o, const vec3& v) const { return 0.0; }
	virtual vec3 random(const vec3& o) const { return vec3(1, 0, 0); }
	virtual ~hittable() {}
};

class bvh_node : public hittable
{
public:
	bvh_node() {}
	bvh_node(hittable **l, int n, double time0, double time1)
	{
		auto comparer_gen = [](int i)
		{
			return [=](hittable* lhs, hittable* rhs)
			{
				aabb box_left, box_right;
				if (
					!lhs->bounding_box(0, 0, box_left) ||
					!rhs->bounding_box(0, 0, box_right))
				{
					std::cerr << "no bouding box in bvh_node constructor\n";
				}
				return box_left.min()[i] < box_right.min()[i];
			};
		};

		// sort along axis
		static std::uniform_int_distribution<int> uniform(0, 2);
		static std::minstd_rand engine;
		int axis = uniform(engine); // get 0, 1, 2
		std::sort(&l[0], &l[n - 1], comparer_gen(axis));

		if (n == 1)
		{
			// left = right
			left = right = l[0];
		}
		else if (n == 2)
		{
			// left, right
			left = l[0];
			right = l[1];
		}
		else
		{
			// left, right as sub-tree recursively
			left = new bvh_node(l, n / 2, time0, time1);
			left = new bvh_node(l +  n / 2, n - n / 2, time0, time1);
		}

		// check and compute aabb
		aabb box_left, box_right;
		if (
			!left->bounding_box(time0, time1, box_left) ||
			!right->bounding_box(time0, time1, box_right))
		{
			std::cerr << "no bouding box in bvh_node constructor\n";
		}
		box = aabb::surrounding(box_left, box_right);
	}

	bool hit(const ray& r, double tmin, double tmax, hit_record& rec) const override
	{
		if (box.hit(r, tmin, tmax))
		{
			hit_record left_rec, right_rec;
			bool hit_left = left->hit(r, tmin, tmax, left_rec);
			bool hit_right = left->hit(r, tmin, tmax, right_rec);

			if (hit_left && hit_right)
			{
				rec = left_rec.t < right_rec.t ? left_rec : right_rec;
				return true;
			}
			else if (hit_left)
			{
				rec = left_rec;
				return true;
			}
			else if (hit_right)
			{
				rec = right_rec;
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	bool bounding_box(double t0, double t1, aabb& b) const override
	{
		b = box;
		return true;
	}

	hittable* left;
	hittable* right;
	aabb box;
};

class xy_rect : public hittable
{
public:
	xy_rect() {}
	xy_rect(double _x0, double _x1, double _y0, double _y1,
		double _k, std::shared_ptr<material> mat) :
			x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat) {};
	bool hit(const ray& r, double t0, double t1, hit_record& rec) const override
	{
		double t = (k - r.origin().z) / r.direction().z;
		if (t < t0 || t > t1)
			return false;
		double x = r.origin().x + t * r.direction().x;
		double y = r.origin().y + t * r.direction().y;
		if (x < x0 || x > x1 || y < y0 || y > y1)
			return false;
		rec.u = (x - x0) / (x1 - x0);
		rec.v = (y - y0) / (y1 - y0);
		rec.t = t;
		rec.mat_ptr = mp.get();
		rec.p = r.point_at_parameter(t);
		rec.normal = vec3(0, 0, 1);
		return true;
	}

	bool bounding_box(double t0, double t1, aabb& box) const override
	{
		box = aabb(vec3(x0, y0, k - 0.0001f), vec3(x1, y1, k + 0.0001f));
		return true;
	}

	double x0, x1, y0, y1, k;
	std::shared_ptr<material> mp;
};

class xz_rect : public hittable
{
public:
	xz_rect() {}
	xz_rect(double _x0, double _x1, double _z0, double _z1,
		double _k, std::shared_ptr<material> mat) :
			x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) {};
	bool hit(const ray& r, double t0, double t1, hit_record& rec) const override
	{
		double t = (k - r.origin().y) / r.direction().y;
		if (t < t0 || t > t1)
			return false;
		double x = r.origin().x + t * r.direction().x;
		double z = r.origin().z + t * r.direction().z;
		if (x < x0 || x > x1 || z < z0 || z > z1)
			return false;
		rec.u = (x - x0) / (x1 - x0);
		rec.v = (z - z0) / (z1 - z0);
		rec.t = t;
		rec.mat_ptr = mp.get();
		rec.p = r.point_at_parameter(t);
		rec.normal = vec3(0, 1, 0);
		return true;
	}

	bool bounding_box(double t0, double t1, aabb& box) const override
	{
		box = aabb(vec3(x0, k - 0.0001f, z0), vec3(x1, k + 0.0001f, z1));
		return true;
	}

	virtual double pdf_value(const vec3& origin, const vec3& v) const override
	{
		// same as hard-coded version in book3.chapter9

		hit_record rec;
		// ensure hit if direction is right by letting 0.001 < FLT_MAX < +inf
		if (!this->hit(ray(origin, v, FLT_MAX), 0.001, std::numeric_limits<double>::infinity(), rec))
			return 0;

		auto area = (x1 - x0) * (z1 - z0);
		auto distance_squared = rec.t * rec.t * v.length_squared();
		auto cosine = fabs(dot(v, rec.normal) / v.length());

		return distance_squared / (cosine * area);
	}

	virtual vec3 random(const vec3& origin) const override
	{
		auto random = vec3(random_double(x0, x1), k, random_double(z0, z1));
		return random - origin;
	}

	double x0, x1, z0, z1, k;
	std::shared_ptr<material> mp;
};

class yz_rect : public hittable
{
public:
	yz_rect() {}
	yz_rect(double _y0, double _y1, double _z0, double _z1,
		double _k, std::shared_ptr<material> mat) :
			y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat) {};
	bool hit(const ray& r, double t0, double t1, hit_record& rec) const override
	{
		double t = (k - r.origin().x) / r.direction().x;
		if (t < t0 || t > t1)
			return false;
		double y = r.origin().y + t * r.direction().y;
		double z = r.origin().z + t * r.direction().z;
		if (y < y0 || y > y1 || z < z0 || z > z1)
			return false;
		rec.u = (y - y0) / (y1 - y0);
		rec.v = (z - z0) / (z1 - z0);
		rec.t = t;
		rec.mat_ptr = mp.get();
		rec.p = r.point_at_parameter(t);
		rec.normal = vec3(1, 0, 0);
		return true;
	}

	bool bounding_box(double t0, double t1, aabb& box) const override
	{
		box = aabb(vec3(k - 0.0001f, y0, z0), vec3(k + 0.0001f, y1, z1));
		return true;
	}

	double y0, y1, z0, z1, k;
	std::shared_ptr<material> mp;
};

class flip_normals : public hittable
{
public:
	flip_normals(std::shared_ptr<hittable> p) : ptr(p) {}
	bool hit(const ray& r, double t0, double t1, hit_record& rec) const override
	{
		if (ptr->hit(r, t0, t1, rec))
		{
			rec.normal = -rec.normal;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool bounding_box(double t0, double t1, aabb& box) const override
	{
		return ptr->bounding_box(t0, t1, box);
	}

	std::shared_ptr<hittable> ptr;
};

// brilliant ! move ray instead of hittable
class translate : public hittable
{
public:
	translate(std::shared_ptr<hittable> p, const vec3& displacement) : ptr(p), offset(displacement) {}
	bool hit(const ray& r, double t0, double t1, hit_record& rec) const override
	{
		ray move_r(r.origin() - offset, r.direction(), r.time());
		if (ptr->hit(move_r, t0, t1, rec))
		{
			rec.p += offset;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool bounding_box(double t0, double t1, aabb& box) const override
	{
		if (ptr->bounding_box(t0, t1, box))
		{
			box = aabb(box.min() + offset, box.max() + offset);
			return true;
		}
		else
		{
			return false;
		}
	}

	std::shared_ptr<hittable> ptr;
	vec3 offset;
};

// rotate hittable
class rotate_y : public hittable
{
public:
	rotate_y(std::shared_ptr<hittable> p, double angle) : ptr(p)
	{
		// calculate new aabb
		double radians = ((double)M_PI / 180.0) * angle;
		sin_theta = sin(radians);
		cos_theta = cos(radians);
		double floatMax = std::numeric_limits<double>::max();
		hasbox = ptr->bounding_box(0, 1, bbox);
		vec3 min(floatMax, floatMax, floatMax);
		vec3 max(-floatMax, -floatMax, -floatMax);
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				for (int k = 0; k < 2; k++)
				{
					double x = i * bbox.max().x + (1 - i) * bbox.min().x;
					double y = i * bbox.max().y + (1 - i) * bbox.min().y;
					double z = i * bbox.max().z + (1 - i) * bbox.min().z;

					double newx = cos_theta * x + sin_theta * z;
					double newz = -sin_theta * x + cos_theta * z;
					vec3 tester(newx, y, newz);
					for (int c = 0; c < 3; c++)
					{
						if (tester[c] > max[c])
						{
							max[c] = tester[c];
						}
						if (tester[c] < min[c])
						{
							min[c] = tester[c];
						}
					}
				}
			}
		}
		bbox = aabb(min, max);
	}
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const
	{
		vec3 origin = r.origin();
		vec3 direction = r.direction();

		origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
		origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];

		direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
		direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

		ray rotated_r(origin, direction, r.time());
		if (ptr->hit(rotated_r, t_min, t_max, rec))
		{
			vec3 p = rec.p;
			vec3 normal = rec.normal;

			p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
			p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

			normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
			normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

			rec.p = p;
			rec.normal = normal;
			return true;
		}
		else
		{
			return false;
		}
	}
	virtual bool bounding_box(double t0, double t1, aabb& box) const
	{
		box = bbox;
		return hasbox;
	}
	virtual ~rotate_y() {}
	std::shared_ptr<hittable> ptr;
	double sin_theta;
	double cos_theta;
	bool hasbox;
	aabb bbox;
};

// probability = C(proportional to optical density) * dL(distance)

class constant_medium : public hittable
{
public:
	constant_medium(
		std::shared_ptr<hittable> b,
		double d,
		std::shared_ptr<material> mat) : boundary(b), density(d), mp(mat)
	{
	}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const
	{
		static std::uniform_real_distribution<double> uniform;
		static std::minstd_rand engine;

		hit_record rec1, rec2;

		// hit the volume
		if (boundary->hit(
			r,
			-std::numeric_limits<double>::max(),
			std::numeric_limits<double>::max(),
			rec1))
		{
			// take a small step
			if(boundary->hit(
				r,
				rec1.t + 0.0001f,
				std::numeric_limits<double>::max(),
				rec2))
			{
				// out of bound
				if (rec1.t < t_min)
					rec1.t = t_min;
				if (rec2.t > t_max)
					rec2.t = t_max;
				if (rec1.t >= rec2.t)
					return false;

				// eliminate minus value
				if (rec1.t < 0)
					rec1.t = 0;

				double distance_inside_boundary =
					(rec2.t - rec1.t) * r.direction().length();
				double hit_distance = -(1 / density) * log(uniform(engine));

				if (hit_distance < distance_inside_boundary)
				{
					rec.t = rec1.t + hit_distance / r.direction().length();
					rec.p = r.point_at_parameter(rec.t);
					rec.normal = vec3(1, 0, 0); // arbitrary
					rec.mat_ptr = mp.get();
					return true;
				}
			}
		}

		return false;
	}

	virtual bool bounding_box(double t0, double t1, aabb& box) const
	{
		return boundary->bounding_box(t0, t1, box);
	}

	std::shared_ptr<hittable> boundary;
	double density;
	std::shared_ptr<material> mp;
};


