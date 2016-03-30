// RayTracingWeekend.cpp : Defines the entry point for the console application.
//

#include <tchar.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <algorithm>

#include <amp.h>                // C++ AMP header file
using namespace concurrency;

#undef min
#undef max
#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))

#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "hitable_list.h"
#include "camera.h"
#include "material.h"

const int nx = 400;
const int ny = 200;
const int subPixelCount = 400;
const int max_depth = 50;

// x : -2  ~  2
// y : -1  ~  1
// z :  0  ~ -1
// center : (0, 0, -1)
// depth is recursion depth...
vec3 color(const ray& r, hitable *world, int depth)
{
	hit_record rec;
	// z_min = 0 will cause hit in the same point -> darker
	if (world->hit(r, 0.001f, FLT_MAX, rec))
	{
		ray scattered;
		vec3 attenuation;

		// * Bounce and reflect depending on material
		if (depth < max_depth && rec.mat_ptr != nullptr && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		{
			return attenuation * color(scattered, world, depth + 1);
		}
		else
		{
			return vec3(0, 0, 0);
		}

		// * Show normal
		{
			// return 0.5f * (rec.normal + 1); 
		}
		

		// * Bounce towards unit sphere above first contact point
		{
			// vec3 random_vector;// = random_in_unit_sphere();
			// vec3 target = rec.p + rec.normal + random_vector;
			// return 0.5f * color(ray(rec.p, target - rec.p), world); // what if bounce never end ? need a limit here
		}		
	}
	else
	{
		// * Gradient background along y-axis
		vec3 unit_direction = normalize(r.direction());
		float t = 0.5f * (unit_direction.y + 1.0f);
		return lerp(vec3(0.5f, 0.7f, 1.0f), vec3(1.0f, 1.0f, 1.0f), t);
	}
}

// https://msdn.microsoft.com/en-us/library/dd728080.aspx
template <class Function>
__int64 time_call(Function&& f)
{
	auto start = std::chrono::high_resolution_clock::now();

	f();

	auto end = std::chrono::high_resolution_clock::now();
	auto diff = end - start;

	return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
}

template <typename _Index_type, typename _Function>
void serial_for(_Index_type _First, _Index_type _Last, _Index_type _Step, const _Function& _Func)
{
	for (_Index_type i = _First; i < _Last; i += _Step)
	{
		_Func(i);
	}
}

template <typename _Index_type, typename _Function>
void _for(_Index_type _First, _Index_type _Last, _Index_type _Step, const _Function& _Func)
{
	//serial_for(_First, _Last, _Step, _Func);
	parallel_for(_First, _Last, _Step, _Func);
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::ofstream out("1.ppm");
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(out.rdbuf()); //redirect std::cout

	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	vec3 lower_left_cornor(-2.0f, -1.0f, -1.0f);
	vec3 horizontal(4.0f, 0.0f, 0.0f);
	vec3 vertical(0.0f, 2.0f, 0.0f);
	vec3 origin(0.0f, 0.0f, 0.0f);

	std::unique_ptr<hitable> list[] =
	{
		std::make_unique<sphere>(vec3(0, 0, -1), 0.5f, 
			std::make_unique<lambertian>(vec3(0.8f, 0.3f, 0.3f))),
		std::make_unique<sphere>(vec3(0, -100.5f, -1), 100.0f, 
			std::make_unique<lambertian>(vec3(0.8f, 0.8f, 0.0f))),
		std::make_unique<sphere>(vec3(1, 0, -1), 0.5f,
			std::make_unique<metal>(vec3(0.8f, 0.6f, 0.2f), 1.0f)),
		std::make_unique<sphere>(vec3(-1, 0, -1), 0.5f,
			std::make_unique<metal>(vec3(0.8f, 0.8f, 0.8f), 0.3f))
	};

	hitable_list world(list, ARRAY_SIZE(list));
	camera cam;

	std::uniform_real<float> uniform;
	std::minstd_rand engine;
	auto canvas = std::unique_ptr<vec3[]>(new vec3[nx * ny]);
	__int64 elapsedTrace = time_call([&]
	{
		// accelerated on this parallel
		_for(0, ny, 1, [&](int j)
		{
			// no obvious acceleration on this parallel
			_for(0, nx, 1, [&](int i)
			{
				vec3 subPixels[subPixelCount];
				_for(0, subPixelCount, 1, [&](int s)
				{
					float u = float(i + uniform(engine)) / float(nx);
					float v = float(j + uniform(engine)) / float(ny);

					// trace
					ray r = cam.get_ray(u, v);
					vec3 p = r.point_at_parameter(2.0f);
					subPixels[s] = color(r, &world, 0);
				});

				vec3 sum(0, 0, 0);
				for (auto& c : subPixels) // even slower with parallel_reduce
				{
					sum += c;
				}

				// to gamma 2
				auto col = sum / subPixelCount;
				col = vec3(sqrt(col.x), sqrt(col.y), sqrt(col.z));

				// save to canvas
				canvas[j * nx + i] = col;
			});
		});
	});

	__int64 elapsedWrite = time_call([&]
	{
		for (int j = ny - 1; j >= 0; j--)
		{
			for (int i = 0; i < nx; i++)
			{
				vec3 col = canvas[j * nx + i];

				int ir = int(255.99f * col.r);
				int ig = int(255.99f * col.g);
				int ib = int(255.99f * col.b);

				std::cout << ir << " " << ig << " " << ib << "\n"; 
			}
		}
	});

	std::cout.rdbuf(coutbuf); //reset to standard output again
	std::cout << "Trace: " << elapsedTrace << std::endl;
	std::cout << "Write: " << elapsedWrite << std::endl;

	out.close();

	// ImageMagick
	system("convert 1.ppm 1.png");
	system("start 1.png");

	return 0;
}