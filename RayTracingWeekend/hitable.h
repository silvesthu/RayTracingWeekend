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
		static std::uniform_real<float> uniform;
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

	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const
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

	virtual bool bounding_box(float t0, float t1, aabb& b) const
	{
		b = box;
		return true;
	}

	hitable* left;
	hitable* right;
	aabb box;
};