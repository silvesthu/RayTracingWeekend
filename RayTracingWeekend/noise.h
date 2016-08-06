#pragma once

#include <algorithm>
#include <random>

#include "vec3.h"
#include "ray.h"

inline float smoothstep_hermite_cubic(float x)
{
	return x * x * (3 - 2 * x);
}

inline float smootherstep_perlin(float x)
{
	return x * x * x * (x * (x * 6 - 15) + 10);
}

#define smooth smoothstep_hermite_cubic
//#define smooth smootherstep_perlin

inline float trilinear_interp(float c[2][2][2], float u, float v, float w)
{
	float uu = smooth(u);
	float vv = smooth(v);
	float ww = smooth(w);

	float accum = 0;
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			for (int k = 0; k < 2; k++)
			{
				accum += (i * uu + (1 - i) * (1 - uu)) *
						 (j * vv + (1 - j) * (1 - vv)) *
						 (k * ww + (1 - k) * (1 - ww)) * c[i][j][k];
			}
	return accum;
}

inline float perlin_interp(vec3 c[2][2][2], float u, float v, float w)
{
	float uu = smooth(u);
	float vv = smooth(v);
	float ww = smooth(w);

	float accum = 0;
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			for (int k = 0; k < 2; k++)
			{
				// basically same as trilinear_interp
				// except this weight applied
				vec3 weight_v(u - i, v - j, w - k);
				accum += (i * uu + (1 - i) * (1 - uu)) *
					(j * vv + (1 - j) * (1 - vv)) *
					(k * ww + (1 - k) * (1 - ww)) * 
						dot(c[i][j][k], weight_v);
			}
	return accum;
}

// Perlin noise
// * repeatable : same input same output
// * near by points return similar numbers
// * simple and fast
// Also see: http://flafla2.github.io/2014/08/09/perlinnoise.html
//
// It seems that only lookup table and & are essential to Perlin noise
// The implementation here followed the raytracing weekend book,
// but is quite different from the official one
class perlin
{
public:
	float turb(const vec3& p, int depth = 7) const
	{
		float accum = 0;
		vec3 temp_p = p;
		float weight = 1.0f;
		for (int i = 0; i < depth; i++)
		{
			accum += weight * noise(temp_p);
			weight *= 0.5f;
			temp_p *= 2;
		}
		return fabs(accum);
	}

	// -1 ~ 1
	float noise(const vec3& p) const 
	{
		if (!initialized)
		{
			initialize();
		}

		float u = p.x - floor(p.x);
		float v = p.y - floor(p.y);
		float w = p.z - floor(p.z);

		enum class InterpolationType
		{
			NONE,
			TRILINEAR,
			PERLIN,
		};

		InterpolationType interpType = InterpolationType::PERLIN;

		switch (interpType)
		{
		case InterpolationType::TRILINEAR:
			{
				int i = (int)floor(p.x);
				int j = (int)floor(p.y);
				int k = (int)floor(p.z);

				float c[2][2][2];

				for (int di = 0; di < 2; di++)
					for (int dj = 0; dj < 2; dj++)
						for (int dk = 0; dk < 2; dk++)
							c[di][dj][dk] = ranfloat[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];

				return trilinear_interp(c, u, v, w);
			}
		case InterpolationType::PERLIN:
			{
				int i = (int)floor(p.x);
				int j = (int)floor(p.y);
				int k = (int)floor(p.z);

				vec3 c[2][2][2];

				for (int di = 0; di < 2; di++)
					for (int dj = 0; dj < 2; dj++)
						for (int dk = 0; dk < 2; dk++)
							c[di][dj][dk] = ranvec[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];

				return perlin_interp(c, u, v, w);
			}
		case InterpolationType::NONE:
		default:
			{
				int i = int(4 * p.x) & 255;
				int j = int(4 * p.y) & 255;
				int k = int(4 * p.z) & 255;

				return ranfloat[perm_x[i] ^ perm_y[j] ^ perm_z[k]];
			}		
		}
	}

private:
	static void initialize()
	{
		perlin_generate_float(ranfloat, SIZE);
		perlin_generate_vec3(ranvec, SIZE);

		perlin_generate_perm(perm_x, SIZE);
		perlin_generate_perm(perm_y, SIZE);
		perlin_generate_perm(perm_z, SIZE);

		initialized = true;
	}

	static void permute(int* p, int n)
	{
		std::uniform_real_distribution<float> uniform;
		std::minstd_rand engine;

		for (int i = n - 1; i > 0; i--)
		{
			int target = int(uniform(engine) * (i + 1));
			int tmp = p[i];
			p[i] = p[target];
			p[target] = tmp;
		}
	}

	static void perlin_generate_perm(int* p, int n)
	{
		for (int i = 0; i < n; i++)
		{
			p[i] = i;
		}
		permute(p, n);
	}

	static void perlin_generate_float(float* p, int n)
	{
		std::uniform_real_distribution<float> uniform;
		std::minstd_rand engine;

		for (int i = 0; i < n; i++)
		{
			p[i] = uniform(engine);
		}
	}

	static void perlin_generate_vec3(vec3* p, int n)
	{
		std::uniform_real_distribution<float> uniform;
		std::minstd_rand engine;

		for (int i = 0; i < n; i++)
		{
			p[i] = vec3(
				-1.0f + 2.0f * uniform(engine),
				-1.0f + 2.0f * uniform(engine),
				-1.0f + 2.0f * uniform(engine));
			p[i] = normalize(p[i]);
		}
	}

	static const int SIZE = 256;
	
	// as lookup table
	static float ranfloat[SIZE];	
	static vec3 ranvec[SIZE];
	
	static int perm_x[SIZE];
	static int perm_y[SIZE];
	static int perm_z[SIZE];

	static bool initialized;
};