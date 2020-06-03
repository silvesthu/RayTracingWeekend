#pragma once

#include "onb.h"
#include "hittable.h"

class pdf
{
public:
	virtual ~pdf() {}

	virtual double value(const vec3& direction) const = 0;
	virtual vec3 generate() const = 0;
};

class cosine_pdf : pdf
{
public:
	cosine_pdf(const vec3& w) { uvw.build_from_w(w); }

	virtual double value(const vec3& direction) const override
	{
		double cosine = dot(normalize(direction), uvw.w());
		return (cosine <= 0) ? 0 : cosine / M_PI;
	}

	virtual vec3 generate() const override
	{
		return uvw.local(random_cosine_direction());
	}

private:
	onb uvw;
};

class hittable_pdf : pdf
{
public:
	hittable_pdf(std::shared_ptr<hittable> p, const vec3& origin) : ptr(p), o(origin) {}

	virtual double value(const vec3& direction) const 
	{
		return ptr->pdf_value(o, direction);
	}

	virtual vec3 generate() const 
	{
		return ptr->random(o);
	}

public:
	vec3 o;
	std::shared_ptr<hittable> ptr;
};