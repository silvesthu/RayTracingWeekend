#pragma once

#include <iostream>

#include "vec3.h"
#include "ray.h"
#include "aabb.h"

class material;

struct hit_record
{
	hit_record()
	{
		mat_ptr = nullptr;
		t = 0;
	}

	float t;
	vec3 p;
	vec3 normal; // should filled with normalized normal
	float u;
	float v;
	material *mat_ptr;
};

class hitable
{
public:
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
	virtual bool bounding_box(float t0, float t1, aabb& box) const = 0;
	virtual ~hitable() {}
};

class bvh_node : public hitable
{
public:
	bvh_node() {}
	bvh_node(hitable **l, int n, float time0, float time1)
	{
		static std::uniform_real_distribution<float> uniform;
		static std::minstd_rand engine;

		auto comparer_gen = [](int i)
		{
			return [=](hitable* lhs, hitable* rhs)
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
		int axis = std::max(int(3 * uniform(engine)), 2); // get 0, 1, 2
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

	bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const override
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

	bool bounding_box(float t0, float t1, aabb& b) const override
	{
		b = box;
		return true;
	}

	hitable* left;
	hitable* right;
	aabb box;
};

class xy_rect : public hitable
{
public:
	xy_rect() {}
	xy_rect(float _x0, float _x1, float _y0, float _y1,
		float _k, std::shared_ptr<material> mat) :
			x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat) {};
	bool hit(const ray& r, float t0, float t1, hit_record& rec) const override
	{
		float t = (k - r.origin().z) / r.direction().z;
		if (t < t0 || t > t1)
			return false;
		float x = r.origin().x + t * r.direction().x;
		float y = r.origin().y + t * r.direction().y;
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

	bool bounding_box(float t0, float t1, aabb& box) const override
	{
		box = aabb(vec3(x0, y0, k - 0.0001f), vec3(x1, y1, k + 0.0001f));
		return true;
	}

	float x0, x1, y0, y1, k;
	std::shared_ptr<material> mp;
};

class xz_rect : public hitable
{
public:
	xz_rect() {}
	xz_rect(float _x0, float _x1, float _z0, float _z1,
		float _k, std::shared_ptr<material> mat) :
			x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) {};
	bool hit(const ray& r, float t0, float t1, hit_record& rec) const override
	{
		float t = (k - r.origin().y) / r.direction().y;
		if (t < t0 || t > t1)
			return false;
		float x = r.origin().x + t * r.direction().x;
		float z = r.origin().z + t * r.direction().z;
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

	bool bounding_box(float t0, float t1, aabb& box) const override
	{
		box = aabb(vec3(x0, k - 0.0001f, z0), vec3(x1, k + 0.0001f, z1));
		return true;
	}

	float x0, x1, z0, z1, k;
	std::shared_ptr<material> mp;
};

class yz_rect : public hitable
{
public:
	yz_rect() {}
	yz_rect(float _y0, float _y1, float _z0, float _z1,
		float _k, std::shared_ptr<material> mat) :
			y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat) {};
	bool hit(const ray& r, float t0, float t1, hit_record& rec) const override
	{
		float t = (k - r.origin().x) / r.direction().x;
		if (t < t0 || t > t1)
			return false;
		float y = r.origin().y + t * r.direction().y;
		float z = r.origin().z + t * r.direction().z;
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

	bool bounding_box(float t0, float t1, aabb& box) const override
	{
		box = aabb(vec3(k - 0.0001f, y0, z0), vec3(k + 0.0001f, y1, z1));
		return true;
	}

	float y0, y1, z0, z1, k;
	std::shared_ptr<material> mp;
};

class flip_normals : public hitable
{
public:
	flip_normals(std::shared_ptr<hitable> p) : ptr(p) {}
	bool hit(const ray& r, float t0, float t1, hit_record& rec) const override
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

	bool bounding_box(float t0, float t1, aabb& box) const override
	{
		return ptr->bounding_box(t0, t1, box);
	}

	std::shared_ptr<hitable> ptr;
};