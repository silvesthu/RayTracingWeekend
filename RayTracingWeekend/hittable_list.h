#pragma once

#include "hittable.h"

class hittable_list : public hittable 
{
public:
	hittable_list() {}
	hittable_list(const std::vector<std::shared_ptr<hittable>>& l) : objects(l) {}

	bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override
	{
		hit_record temp_rec;
		bool hit_anything = false;
		double closet_so_far = t_max;
		for (size_t i = 0; i < objects.size(); i++)
		{
			if (objects[i]->hit(r, t_min, closet_so_far, temp_rec))
			{
				hit_anything = true;
				closet_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}

		for (auto& p : objects)
		{
			if (p->hit(r, t_min, closet_so_far, temp_rec))
			{
				hit_anything = true;
				closet_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}

		return hit_anything;
	}

	bool bounding_box(double t0, double t1, aabb& box) const override
	{
		return true;
	}

	double hittable_list::pdf_value(const vec3& o, const vec3& v) const override
	{
		double weight = 1.0 / objects.size();
		double sum = 0.0;

		for (const auto& object : objects)
			sum += weight * object->pdf_value(o, v);

		return sum;
	}

	vec3 hittable_list::random(const vec3& o) const override
	{
		int int_size = static_cast<int>(objects.size());
		return objects[random_int(0, int_size - 1)]->random(o);
	}

	std::vector<std::shared_ptr<hittable>> objects;
};

// AABB
class box : public hittable
{
public:
	box() {}
	box(const vec3& p0, const vec3& p1, std::shared_ptr<material> mat)
	{
		pmin = p0;
		pmax = p1;

		// AABB composed with 6 planes
		std::vector<std::shared_ptr<hittable>> objects;
		objects.push_back(
			std::make_shared<xy_rect>(
				p0.x, p1.x, p0.y, p1.y, p1.z, mat));
		objects.push_back(std::make_shared<flip_normals>(
			std::make_shared<xy_rect>(
				p0.x, p1.x, p0.y, p1.y, p0.z, mat)));

		objects.push_back(
			std::make_shared<xz_rect>(
				p0.x, p1.x, p0.z, p1.z, p1.y, mat));
		objects.push_back(std::make_shared<flip_normals>(
			std::make_shared<xz_rect>(
				p0.x, p1.x, p0.z, p1.z, p0.y, mat)));

		objects.push_back(
			std::make_shared<yz_rect>(
				p0.y, p1.y, p0.z, p1.z, p1.x, mat));
		objects.push_back(std::make_shared<flip_normals>(
			std::make_shared<yz_rect>(
				p0.y, p1.y, p0.z, p1.z, p0.x, mat)));

		list_ptr = hittable_list(objects);
	}

	bool hit(const ray& r, double t0, double t1, hit_record& rec) const override
	{
		// test hit with each plane
		return list_ptr.hit(r, t0, t1, rec);
	}

	bool bounding_box(double t0, double t1, aabb& box) const override
	{
		box = aabb(pmin, pmax);
		return true;
	}

	vec3 pmin, pmax;
	hittable_list list_ptr; // hold 6 planes
};
