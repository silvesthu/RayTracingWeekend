#pragma once

#include "hitable.h"

class hitable_list : public hitable {
public:
	hitable_list() {}
	hitable_list(std::unique_ptr<hitable>*l, int n) { list = l; list_size = n; }
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override
	{
		hit_record temp_rec;
		bool hit_anything = false;
		float closet_so_far = t_max;
		for (int i = 0; i < list_size; i++)
		{
			if (list[i]->hit(r, t_min, closet_so_far, temp_rec))
			{
				hit_anything = true;
				closet_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}
		return hit_anything;
	}

	virtual bool bounding_box(float t0, float t1, aabb& box) const
	{
		return true;
	}

	std::unique_ptr<hitable>* list;
	int list_size;
};