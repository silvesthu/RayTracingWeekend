#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include <cstdlib>
#include <algorithm>

class vec3
{
public:
	vec3() { x = 0; y = 0; z = 0; } // initialize it in case...

	// double -> vec3 convesion
	vec3(double t) { x = t; y = t; z = t; }

	vec3(double e0, double e1, double e2) { x = e0; y = e1; z = e2; }

	inline const vec3& operator+() const { return *this; }
	inline vec3 operator-() const { return vec3(-x, -y, -z); }
	inline double operator[](int i) const { return e[i]; }
	inline double& operator[](int i) { return e[i]; }

	inline vec3& operator+=(const vec3& v2) { x += v2.x; y += v2.y; z += v2.z; return *this; }
	inline vec3& operator-=(const vec3& v2) { x -= v2.x; y -= v2.y; z -= v2.z; return *this; }
	inline vec3& operator*=(const vec3& v2) { x *= v2.x; y *= v2.y; z *= v2.z; return *this; }
	inline vec3& operator/=(const vec3& v2) { x /= v2.x; y /= v2.y; z /= v2.z; return *this; }
	inline vec3& operator*=(const double t) { x *= t; y *= t; z *= t; return *this; }
	inline vec3& operator/=(const double t) { x /= t; y /= t; z /= t; return *this; }

	inline double length_squared() const { return x * x + y * y + z * z; }
	inline double length() const { return std::sqrt(length_squared()); }
	inline void make_unit_vector();

	union
	{
		struct
		{
			union { double x; double r; };
			union { double y; double g; };
			union { double z; double b; };
		};
		double e[3];
	};
};

inline vec3 operator+(const vec3& v1, const vec3& v2) { vec3 copy = v1; copy += v2; return copy; }
inline vec3 operator-(const vec3& v1, const vec3& v2) { vec3 copy = v1; copy -= v2; return copy; }
inline vec3 operator*(const vec3& v1, const vec3& v2) { vec3 copy = v1; copy *= v2; return copy; }
inline vec3 operator/(const vec3& v1, const vec3& v2) { vec3 copy = v1; copy /= v2; return copy; }

inline double dot(const vec3& v1, const vec3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

inline vec3 cross(const vec3& v1, const vec3& v2)
{
	return vec3((v1.y * v2.z - v1.z * v2.y),
				(-(v1.x * v2.z - v1.z * v2.x)),
				(v1.x * v2.y - v1.y * v2.x));
}

inline vec3 normalize(const vec3& v)
{
	vec3 copy = v;
	double length = copy.length();
	copy /= length;
	return copy;
}

template <typename T>
inline T clamp(const T& x, const T& min, const T& max)
{
	return std::max(std::min(x, max), min);
}

inline vec3 clamp(const vec3& v, const vec3& min, const vec3& max)
{
	vec3 r = v;
	r.x = clamp(v.x, min.x, max.x);
	r.y = clamp(v.y, min.y, max.y);
	r.z = clamp(v.z, min.z, max.z);
	return r;
}

inline vec3 lerp(vec3 from, vec3 to, double t)
{
	return (1.0 - t) * to + t * from;
}

inline void vec3::make_unit_vector()
{
	*this = normalize(*this);
}