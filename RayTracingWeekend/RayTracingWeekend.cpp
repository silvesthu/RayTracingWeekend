// RayTracingWeekend.cpp : Defines the entry point for the console application.
//

#define NOMINMAX

#include <tchar.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <ppl.h>
using namespace concurrency;

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))

#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "hitable_list.h"
#include "camera.h"
#include "material.h"

#ifdef _DEBUG
	const int size_multipler = 1;
	const int subPixel_count = 5;
#else
	const int size_multipler = 2;
	const int subPixel_count = 5;
#endif

const int nx = 200 * size_multipler;
const int ny = 100 * size_multipler;
const int subPixelCount = subPixel_count * size_multipler;

//#define DEBUG_RAY

#ifdef DEBUG_RAY
const int max_depth = 1;
#else
const int max_depth = 50;
#endif

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
	Concurrency::parallel_for(_First, _Last, _Step, _Func);
}

int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	std::ofstream out("1.ppm");
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(out.rdbuf()); //redirect std::cout

	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	// Debug Scene
	//std::unique_ptr<hitable> list[] =
	//{
	//	std::make_unique<sphere>(vec3(0, 0, -1), 0.5f,
	//		std::make_unique<metal>(vec3(0.8f, 0.6f, 0.2f), 0.0f))
	//};

	// Metal scene
	//std::unique_ptr<hitable> list[] =
	//{
	//	std::make_unique<sphere>(vec3(0, 0, -1), 0.5f, 
	//		std::make_unique<lambertian>(vec3(0.8f, 0.3f, 0.3f))),
	//	std::make_unique<sphere>(vec3(0, -100.5f, -1), 100.0f, 
	//		std::make_unique<lambertian>(vec3(0.8f, 0.8f, 0.0f))),
	//	std::make_unique<sphere>(vec3(1, 0, -1), 0.5f,
	//		std::make_unique<metal>(vec3(0.8f, 0.6f, 0.2f), 1.0f)),
	//	std::make_unique<sphere>(vec3(-1, 0, -1), 0.5f,
	//		std::make_unique<metal>(vec3(0.8f, 0.8f, 0.8f), 0.3f))
	//};
	
	// Dielectric scene
	//std::unique_ptr<hitable> list[] =
	//{
	//	std::make_unique<sphere>(vec3(0, 0, -1), 0.5f, 
	//		std::make_unique<lambertian>(vec3(0.1f, 0.2f, 0.5f))),
	//	std::make_unique<sphere>(vec3(0, -100.5f, -1), 100.0f, 
	//		std::make_unique<lambertian>(vec3(0.8f, 0.8f, 0.0f))),
	//	std::make_unique<sphere>(vec3(1, 0, -1), 0.5f,
	//		std::make_unique<metal>(vec3(0.8f, 0.6f, 0.2f), 0.0f)),
	//	std::make_unique<sphere>(vec3(-1, 0, -1), 0.5f,
	//		std::make_unique<dielectric>(1.5f))
	//};

	// Dielectric scene - schlick
	//std::unique_ptr<hitable> list[] =
	//{
	//	std::make_unique<sphere>(vec3(0, 0, -1), 0.5f, 
	//		std::make_unique<lambertian>(vec3(0.1f, 0.2f, 0.5f))),
	//	std::make_unique<sphere>(vec3(0, -100.5f, -1), 100.0f, 
	//		std::make_unique<lambertian>(vec3(0.8f, 0.8f, 0.0f))),
	//	std::make_unique<sphere>(vec3(1, 0, -1), 0.5f,
	//		std::make_unique<metal>(vec3(0.8f, 0.6f, 0.2f), 0.0f)),
	//	std::make_unique<sphere>(vec3(-1, 0, -1), 0.5f,
	//		std::make_unique<dielectric>(1.5f)),
	//	std::make_unique<sphere>(vec3(-1, 0, -1), -0.45f,
	//		std::make_unique<dielectric>(1.5f))
	//};

	// Camera test scene
	//float R = cos((float)M_PI / 4.0f);
	//std::unique_ptr<hitable> list[] =
	//{
	//	std::make_unique<sphere>(vec3(-R, 0, -1), R, 
	//		std::make_unique<lambertian>(vec3(0, 0, 1))),
	//	std::make_unique<sphere>(vec3(R, 0, -1), R, 
	//		std::make_unique<lambertian>(vec3(1, 0, 0)))
	//};

	//int n = ARRAY_SIZE(list);

	// Random scene
	const int n = 480 + 1 + 3;
	std::unique_ptr<hitable> list[n];
	{
		// The ground
		std::unique_ptr<texture> groundAlbedo0 = std::make_unique<constant_texture>(vec3(0.2f, 0.3f, 0.1f));
		std::unique_ptr<texture> groundAlbedo1 = std::make_unique<constant_texture>(vec3(0.9f, 0.9f, 0.9f));
		std::unique_ptr<texture> groundAlbedo = std::make_unique<checker_texture>(groundAlbedo0, groundAlbedo1);
		list[0] = std::make_unique<sphere>(vec3(0, -1000, 0), 1000.0f, std::make_unique<lambertian>(groundAlbedo));

		std::uniform_real<float> scene_uniform;
		std::minstd_rand scene_engine;
		std::uniform_real<float> time_uniform;
		std::minstd_rand time_engine;

		// Small ones
		auto rand = [&](){ return scene_uniform(scene_engine); };
		int i = 1;
		for (int a = -11; a < 11; a++)
		{
			for (int b = -11; b < 11; b++)
			{
				float choose_mat = rand();
				vec3 center(a + 0.9f * rand(), 0.2f, b + 0.9f * rand());
				if ((center - vec3(4.0f, 0.2f, 0.0f)).length() > 0.9f)
				{
					if (choose_mat < 0.8f) // diffuse
					{
						vec3 color;
						color.r = rand() * rand();
						color.g = rand() * rand();
						color.b = rand() * rand();
						std::unique_ptr<texture> albedo = std::make_unique<constant_texture>(color);
						list[i++] = std::make_unique<moving_sphere>(center, 0.2f, std::make_unique<lambertian>(albedo));

						moving_strategy s;
						s.center1 = center + vec3(0.0f, 0.5f * time_uniform(time_engine), 0.0f);
						s.time0 = 0.0f;
						s.time1 = 1.0f;
						static_cast<moving_sphere*>(list[i - 1].get())->update_strategy(s);
					} 
					else
					{
						if (choose_mat < 0.95) // metal
						{
							vec3 color;
							color.r = 0.5f * (1 + rand());
							color.g = 0.5f * (1 + rand());
							color.b = 0.5f * (1 + rand());
							float fuzz = 0.5f * rand();
							list[i++] = std::make_unique<sphere>(center, 0.2f, std::make_unique<metal>(color, fuzz));
						}
						else // glass
						{
							vec3 color;
							float ref_idx = 1.5f;
							list[i++] = std::make_unique<sphere>(center, 0.2f, std::make_unique<dielectric>(ref_idx));
						}
					}
				}
			}
		}

		// Big ones
		list[i++] = std::make_unique<sphere>(vec3(0,1,0), 1.0f, std::make_unique<dielectric>(1.5f));
		std::unique_ptr<texture> albedo = std::make_unique<constant_texture>(vec3(0.4f, 0.2f, 0.1f));
		list[i++] = std::make_unique<sphere>(vec3(-4,1,0), 1.0f, std::make_unique<lambertian>(albedo));
		list[i++] = std::make_unique<sphere>(vec3(4,1,0), 1.0f, std::make_unique<metal>(vec3(0.7f, 0.6f, 0.5f), 0.0f));
	}	

	hitable_list world(list, n);
	//vec3 lookfrom(-2, 2, 1); // before defocus
	//vec3 lookfrom(3, 3, 2); // defocus
	//vec3 lookat(0, 0, -1);

	vec3 lookfrom(12, 2, 3);
	vec3 lookat(0, 0.5f, 0);
	float dist_to_focus = (lookfrom - lookat).length();
	float aperture = 0.2f;
	float vfov = 20.0f;
	camera cam(lookfrom, lookat, vec3(0, 1, 0), vfov, float(nx) / float(ny), aperture, dist_to_focus, 0.0f, 1.0f);

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

#ifdef DEBUG_RAY
					// DEBUG_RAY
					u = v = 0.5f;
#endif

					// trace
					ray r = cam.get_ray(u, v);
					subPixels[s] = color(r, &world, 0);
				});

				vec3 sum(0, 0, 0);
				for (auto& c : subPixels) // even slower with parallel_reduce
				{
					sum += c;
				}

				// to gamma 2
				auto col = sum / static_cast<float>(subPixelCount);
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