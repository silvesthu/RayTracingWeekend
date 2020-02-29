#define NOMINMAX

#include <crtdbg.h>
#include <ppl.h>
using namespace concurrency;

#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))

#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "hitable_list.h"
#include "camera.h"
#include "material.h"

#include "Scene\scene.h"
#include "Scene\cornell_box.h"

#include "MonteCarlo\montecarlo.h"

const int size_multipler = 4;
const int subPixelCount = 10;

const int nx = 100 * size_multipler;
const int ny = 100 * size_multipler;

//#define DEBUG_RAY
#ifdef DEBUG_RAY
const int max_depth = 1;
#else
const int max_depth = 100;
#endif

vec3 color(const ray& r, const scene *s, int recursion_depth)
{
	hit_record rec;
	// z_min = 0 will cause hit same point while reflection
	if (s->GetWorld().hit(r, 0.001f, std::numeric_limits<float>::max(), rec))
	{
		ray scattered;
		vec3 attenuation;
		vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

		switch (s->GetRenderType())
		{
		case RenderType::Shaded:
			if (recursion_depth < max_depth && rec.mat_ptr != nullptr && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			{
				return emitted + attenuation * color(scattered, s, recursion_depth + 1);
			}
			else
			{
				return emitted;
			}
		case RenderType::Normal:
			return 0.5f * (rec.normal + 1);
		default:
			return vec3(0, 0, 0);
		}
	}
	else
	{
		switch (s->GetBackgroundType())
		{
			case BackgroundType::Gradient:
			{
				// Gradient background along y-axis
				vec3 unit_direction = normalize(r.direction());
				float t = 0.5f * (unit_direction.y + 1.0f);
				return lerp(vec3(0.5f, 0.7f, 1.0f), vec3(1.0f, 1.0f, 1.0f), t);
			}
			case BackgroundType::Black:
			default:
			{
				// Black background
				return vec3(0, 0, 0);
			}
		}
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
	// Parallel gives different result every time as RNG should not used across threads
	// Switch to Serial to get stable result

	//Concurrency::parallel_for(_First, _Last, _Step, _Func);
	serial_for(_First, _Last, _Step, _Func);
}

int main(int argc, char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// hijack main to test pdf
	//MonteCarlo();
	//return 0;

	typedef dielectric_scene scene_type;
	//typedef cornell_box scene_type;
	//typedef light_sample scene_type;

	scene_type scene(nx * 1.0f / ny);
	auto& cam = scene.GetCamera();

	std::uniform_real_distribution<float> uniform;
	std::minstd_rand engine;

	std::vector<vec3> canvas(nx * ny);
	__int64 elapsedTrace = time_call([&]
	{
		// random function is not thread-safe... -> parallism may affect distribution
		_for(0, ny, 1, [&](int j)
		{
			_for(0, nx, 1, [&](int i)
			{
				vec3 subPixels[subPixelCount];
				_for(0, subPixelCount, 1, [&](int s)
				{
#ifdef DEBUG_RAY
					// DEBUG_RAY point at center
					i = nx / 2;
					j = ny / 2;
#endif

					float u = float(i + uniform(engine)) / float(nx);
					float v = float(j + uniform(engine)) / float(ny);

					// trace
					ray r = cam.get_ray(u, v);
					subPixels[s] = color(r, &scene, 0);
				});

				vec3 sum(0, 0, 0);
				for (auto& c : subPixels) // even slower with parallel_reduce
				{
					sum += c;
				}

				auto col = sum / static_cast<float>(subPixelCount);

				// to gamma 2, and clamp
				col = vec3(std::min(sqrt(col.x), 1.0f), std::min(sqrt(col.y), 1.0f), std::min(sqrt(col.z), 1.0f));

				// save to canvas
				canvas[j * nx + i] = col;
			});
		});
	});

	std::ofstream out("x64\\1.ppm");
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(out.rdbuf()); //redirect std::cout

	// output as ppm
	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	__int64 elapsedWrite = time_call([&]
	{
		for (int j = ny - 1; j >= 0; j--)
		{
			for (int i = 0; i < nx; i++)
			{
				vec3 col = canvas[j * nx + i];

				// 255.99f for float inaccuracy
				int ir = int(255.99f * col.r);
				int ig = int(255.99f * col.g);
				int ib = int(255.99f * col.b);

				// output as ppm
				std::cout << ir << " " << ig << " " << ib << "\n";
			}
		}
	});

	std::cout.rdbuf(coutbuf); // reset to standard output again
	std::cout << "Trace: " << elapsedTrace << "ms" << std::endl;
	std::cout << "Write: " << elapsedWrite << "ms" << std::endl;

	out.close();

	// PPM -> PNG
	system("magick x64\\1.ppm x64\\1.png");
	system("start x64\\1.png");

	return 0;
}