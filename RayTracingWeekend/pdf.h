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

class cosine_pdf : public pdf
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

class hittable_pdf : public pdf
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

class mixture_pdf : public pdf
{
public:
	mixture_pdf(std::shared_ptr<pdf> p0, std::shared_ptr<pdf> p1)
	{
		p[0] = p0;
		p[1] = p1;
	}

	virtual double value(const vec3& direction) const
	{
		return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
	}

	virtual vec3 generate() const
	{
		if (random_double() < 0.5)
			return p[0]->generate();
		else
			return p[1]->generate();
	}

private:
	std::shared_ptr<pdf> p[2];
};