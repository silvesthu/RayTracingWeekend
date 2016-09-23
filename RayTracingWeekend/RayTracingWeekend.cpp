// * Well, shared_ptr is really a mess... but never mind...
// * Benefit from parallism is hard to image (too much will do harm to overall performance)
// * Introduce SIMD in a small proportion make things even worse. Rewrite is time-consuming
// * Amazingly ppl and tbb is easy enough to exchange
// * Always be careful about color range. Overflow made green turns purple in my experiment...

// # 800(w) * 400(h) * 800(sub) => 120s

#define NOMINMAX

#ifdef _WIN32
	#include <crtdbg.h>
	#include <ppl.h>
	using namespace concurrency;
#else
	#include <tbb/tbb.h>
#endif

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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const int size_multipler = 2;
const int subPixel_base_count = 800;

const int nx = 100 * size_multipler;
const int ny = 100 * size_multipler;
const int subPixelCount = subPixel_base_count;

//#define DEBUG_RAY
#ifdef DEBUG_RAY
const int max_depth = 1;
#else
const int max_depth = 100;
#endif

enum class RenderType
{
	Shaded,
	Normal,
};

RenderType s_renderType = RenderType::Shaded;

enum class BackgroundType
{
	Black,
	Gradient,
};

BackgroundType s_backgroundType = BackgroundType::Black;

// x : -2  ~  2
// y : -1  ~  1
// z :  0  ~ -1
// center : (0, 0, -1)
// depth is recursion depth...
vec3 color(const ray& r, hitable *world, int depth)
{
	hit_record rec;
	// z_min = 0 will cause hit in the same point -> darker
	if (world->hit(r, 0.001f, std::numeric_limits<float>::max(), rec))
	{
		ray scattered;
		vec3 attenuation;
		vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

		switch (s_renderType)
		{
		case RenderType::Shaded:
			if (depth < max_depth && rec.mat_ptr != nullptr && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			{
				return emitted + attenuation * color(scattered, world, depth + 1);
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
		switch (s_backgroundType)
		{
			case BackgroundType::Gradient:
			{
				// * Gradient background along y-axis
				vec3 unit_direction = normalize(r.direction());
				float t = 0.5f * (unit_direction.y + 1.0f);
				return lerp(vec3(0.5f, 0.7f, 1.0f), vec3(1.0f, 1.0f, 1.0f), t);
			}
			case BackgroundType::Black:
			default:
			{
				// * Black background
				return vec3(0, 0, 0);
			}
		}
	}
}

#ifndef _WIN32
typedef __int64_t __int64;
#endif

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
#ifdef _WIN32
	Concurrency::parallel_for(_First, _Last, _Step, _Func);
#else
	// only 2 times faster...
	tbb::parallel_for(_First, _Last, _Step, _Func);
#endif
	//serial_for(_First, _Last, _Step, _Func);
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	vec3 lookfrom(12, 2, 3);
	vec3 lookat(0, 0.5f, 0);
	float dist_to_focus = (lookfrom - lookat).length();
	float aperture = 0.2f;
	float vfov = 20.0f;

	// Debug Scene
	//std::shared_ptr<hitable> list[] =
	//{
	//	std::make_shared<sphere>(vec3(0, 0, -1), 0.5f,
	//		std::make_shared<metal>(vec3(0.8f, 0.6f, 0.2f), 0.0f))
	//};

	// Metal scene
	//std::shared_ptr<hitable> list[] =
	//{
	//	std::make_shared<sphere>(vec3(0, 0, -1), 0.5f,
	//		std::make_shared<lambertian>(vec3(0.8f, 0.3f, 0.3f))),
	//	std::make_shared<sphere>(vec3(0, -100.5f, -1), 100.0f,
	//		std::make_shared<lambertian>(vec3(0.8f, 0.8f, 0.0f))),
	//	std::make_shared<sphere>(vec3(1, 0, -1), 0.5f,
	//		std::make_shared<metal>(vec3(0.8f, 0.6f, 0.2f), 1.0f)),
	//	std::make_shared<sphere>(vec3(-1, 0, -1), 0.5f,
	//		std::make_shared<metal>(vec3(0.8f, 0.8f, 0.8f), 0.3f))
	//};

	// Dielectric scene
	//std::shared_ptr<hitable> list[] =
	//{
	//	std::make_shared<sphere>(vec3(0, 0, -1), 0.5f,
	//		std::make_shared<lambertian>(vec3(0.1f, 0.2f, 0.5f))),
	//	std::make_shared<sphere>(vec3(0, -100.5f, -1), 100.0f,
	//		std::make_shared<lambertian>(vec3(0.8f, 0.8f, 0.0f))),
	//	std::make_shared<sphere>(vec3(1, 0, -1), 0.5f,
	//		std::make_shared<metal>(vec3(0.8f, 0.6f, 0.2f), 0.0f)),
	//	std::make_shared<sphere>(vec3(-1, 0, -1), 0.5f,
	//		std::make_shared<dielectric>(1.5f))
	//};

	// Dielectric scene - schlick
	//std::shared_ptr<hitable> list[] =
	//{
	//	std::make_shared<sphere>(vec3(0, 0, -1), 0.5f,
	//		std::make_shared<lambertian>(vec3(0.1f, 0.2f, 0.5f))),
	//	std::make_shared<sphere>(vec3(0, -100.5f, -1), 100.0f,
	//		std::make_shared<lambertian>(vec3(0.8f, 0.8f, 0.0f))),
	//	std::make_shared<sphere>(vec3(1, 0, -1), 0.5f,
	//		std::make_shared<metal>(vec3(0.8f, 0.6f, 0.2f), 0.0f)),
	//	std::make_shared<sphere>(vec3(-1, 0, -1), 0.5f,
	//		std::make_shared<dielectric>(1.5f)),
	//	std::make_shared<sphere>(vec3(-1, 0, -1), -0.45f,
	//		std::make_shared<dielectric>(1.5f))
	//};

	// Camera test scene
	//float R = cos((float)M_PI / 4.0f);
	//std::shared_ptr<hitable> list[] =
	//{
	//	std::make_shared<sphere>(vec3(-R, 0, -1), R,
	//		std::make_shared<lambertian>(vec3(0, 0, 1))),
	//	std::make_shared<sphere>(vec3(R, 0, -1), R,
	//		std::make_shared<lambertian>(vec3(1, 0, 0)))
	//};

	//int n = ARRAY_SIZE(list);

	// Random scene
	//const int n = 480 + 1 + 3;
	//std::shared_ptr<hitable> list[n];
	//{
	//	// The ground
	//	std::shared_ptr<texture> groundAlbedo0 = std::make_shared<constant_texture>(vec3(0.2f, 0.3f, 0.1f));
	//	std::shared_ptr<texture> groundAlbedo1 = std::make_shared<constant_texture>(vec3(0.9f, 0.9f, 0.9f));
	//	std::shared_ptr<texture> groundAlbedo = std::make_shared<checker_texture>(groundAlbedo0, groundAlbedo1);
	//	list[0] = std::make_shared<sphere>(vec3(0, -1000, 0), 1000.0f, std::make_shared<lambertian>(groundAlbedo));

	//	std::uniform_real_distribution<float> scene_uniform;
	//	std::minstd_rand scene_engine;
	//	std::uniform_real_distribution<float> time_uniform;
	//	std::minstd_rand time_engine;

	//	// Small ones
	//	auto rand = [&](){ return scene_uniform(scene_engine); };
	//	int i = 1;
	//	for (int a = -11; a < 11; a++)
	//	{
	//		for (int b = -11; b < 11; b++)
	//		{
	//			float choose_mat = rand();
	//			vec3 center(a + 0.9f * rand(), 0.2f, b + 0.9f * rand());
	//			if ((center - vec3(4.0f, 0.2f, 0.0f)).length() > 0.9f)
	//			{
	//				if (choose_mat < 0.8f) // diffuse
	//				{
	//					vec3 color;
	//					color.r = rand() * rand();
	//					color.g = rand() * rand();
	//					color.b = rand() * rand();
	//					std::shared_ptr<texture> albedo = std::make_shared<constant_texture>(color);
	//					list[i++] = std::make_shared<moving_sphere>(center, 0.2f, std::make_shared<lambertian>(albedo));

	//					moving_strategy s;
	//					s.center1 = center + vec3(0.0f, 0.5f * time_uniform(time_engine), 0.0f);
	//					s.time0 = 0.0f;
	//					s.time1 = 1.0f;
	//					static_cast<moving_sphere*>(list[i - 1].get())->update_strategy(s);
	//				}
	//				else
	//				{
	//					if (choose_mat < 0.95) // metal
	//					{
	//						vec3 color;
	//						color.r = 0.5f * (1 + rand());
	//						color.g = 0.5f * (1 + rand());
	//						color.b = 0.5f * (1 + rand());
	//						float fuzz = 0.5f * rand();
	//						list[i++] = std::make_shared<sphere>(center, 0.2f, std::make_shared<metal>(color, fuzz));
	//					}
	//					else // glass
	//					{
	//						vec3 color;
	//						float ref_idx = 1.5f;
	//						list[i++] = std::make_shared<sphere>(center, 0.2f, std::make_shared<dielectric>(ref_idx));
	//					}
	//				}
	//			}
	//		}
	//	}

	//	// Big ones
	//	list[i++] = std::make_shared<sphere>(vec3(0,1,0), 1.0f, std::make_shared<dielectric>(1.5f));
	//	std::shared_ptr<texture> albedo = std::make_shared<constant_texture>(vec3(0.4f, 0.2f, 0.1f));
	//	list[i++] = std::make_shared<sphere>(vec3(-4,1,0), 1.0f, std::make_shared<lambertian>(albedo));
	//	list[i++] = std::make_shared<sphere>(vec3(4,1,0), 1.0f, std::make_shared<metal>(vec3(0.7f, 0.6f, 0.5f), 0.0f));
	//}


	// 2 sphere scene
	//const int n = 2;
	//std::shared_ptr<hitable> list[n];
	//{
	//	std::shared_ptr<texture> a = std::make_shared<constant_texture>(vec3(0.2f, 0.3f, 0.1f));
	//	std::shared_ptr<texture> b = std::make_shared<constant_texture>(vec3(0.9f, 0.9f, 0.9f));
	//	std::shared_ptr<texture> checker = std::make_shared<checker_texture>(a, b);

	//	list[0] = std::make_shared<sphere>(vec3(0.0f, -10.0f, 0.0f), 10.0f, std::make_shared<lambertian>(checker));
	//	list[1] = std::make_shared<sphere>(vec3(0.0f,  10.0f, 0.0f), 10.0f, std::make_shared<lambertian>(checker));
	//}

	//// 2 perlin sphere scene + emit test
	// const int n = 2;
	// std::shared_ptr<hitable> list[n];
	// {
	// 	std::shared_ptr<texture> noise = std::make_shared<noise_texture>();
	// 	std::shared_ptr<texture> red = std::make_shared<constant_texture>(vec3(1.0f, 0.0f, 0.0f));

	// 	list[0] = std::make_shared<sphere>(vec3(0.0f, -1000.0f, 0.0f), 1000.0f,
	// 		std::make_shared<lambertian>(noise));
	// 	list[1] = std::make_shared<sphere>(vec3(0.0f,  2.0f, 0.0f), 2.0f,
	// 		//std::make_shared<lambertian>(noise));
	// 		std::make_shared<diffuse_light>(red));
	// }

	//// image scene
	//const int n = 1;
	// std::shared_ptr<hitable> list[n];
	// {
	// 	int nx, ny, nn;
	// 	unsigned char* tex_data = stbi_load("earth.jpg", &nx, &ny, &nn, 0);
	// 	auto size = nx * ny * 3;
	// 	auto pArray = std::make_shared<image_texture::byte_array>(&tex_data[0], &tex_data[size]);
	// 	STBI_FREE(tex_data);
	// 	std::cout << "Size: " << nx << " " << ny << " " << nn << " " << size << std::endl;

	// 	std::shared_ptr<texture> image = std::make_shared<image_texture>(pArray, nx, ny);

	// 	list[0] = std::make_shared<sphere>(vec3(0.0f, 0.5f, 0.0f), 2.0f, std::make_shared<lambertian>(image));
	// }

	//// sample light scene
	// const int n = 4;
	// std::shared_ptr<hitable> list[n];
	// {
	// 	std::shared_ptr<texture> pertext = std::make_shared<noise_texture>(4);
	// 	std::shared_ptr<texture> four = std::make_shared<constant_texture>(vec3(4, 4, 4)); // color with scale

	// 	list[0] = std::make_shared<sphere>(vec3(0, -1000, 0), 1000,
	// 		std::make_shared<lambertian>(pertext));
	// 	list[1] = std::make_shared<sphere>(vec3(0, 2, 0), 2,
	// 	 	std::make_shared<lambertian>(pertext));
	// 	list[2] = std::make_shared<sphere>(vec3(0, 7, 0), 2,
	// 		std::make_shared<diffuse_light>(four));
	// 	list[3] = std::make_shared<xy_rect>(3, 5, 1, 3, -2,
	// 		std::make_shared<diffuse_light>(four));
	//
	//	lookat.y += 2;
	//	lookfrom *= 2;
	// }

	// cornell box
	const int n = 8;
	std::shared_ptr<hitable> list[n];
	{
		std::shared_ptr<texture> red_tex = std::make_shared<constant_texture>(vec3(0.65f, 0.05f, 0.05f));
		auto red = std::make_shared<lambertian>(red_tex);
		std::shared_ptr<texture> white_tex = std::make_shared<constant_texture>(vec3(0.73f, 0.73f, 0.73f));
		auto white = std::make_shared<lambertian>(white_tex);
		std::shared_ptr<texture> green_tex = std::make_shared<constant_texture>(vec3(0.12f, 0.45f, 0.15f));
		auto green = std::make_shared<lambertian>(green_tex);
		//std::shared_ptr<texture> light_tex = std::make_shared<constant_texture>(vec3(15.0f, 15.0f, 15.0f));
		std::shared_ptr<texture> light_tex = std::make_shared<constant_texture>(vec3(7.0f, 7.0f, 7.0f));
		auto light = std::make_shared<diffuse_light>(light_tex);

		//list[0] =
		// 	std::make_shared<xz_rect>(213.0f, 343.0f, 227.0f, 332.0f, 554.0f, light);
		list[0] =
			std::make_shared<xz_rect>(113.0f, 443.0f, 127.0f, 432.0f, 554.0f, light);

		list[1] = std::make_shared<flip_normals>(
			std::make_shared<yz_rect>(0.0f, 555.0f, 0.0f, 555.0f, 555.0f, green));
		list[2] =
			std::make_shared<yz_rect>(0.0f, 555.0f, 0.0f, 555.0f, 0.0f, red);

		list[3] = std::make_shared<flip_normals>(
			std::make_shared<xz_rect>(0.0f, 555.0f, 0.0f, 555.0f, 555.0f, white));
		list[4] =
			std::make_shared<xz_rect>(0.0f, 555.0f, 0.0f, 555.0f, 0.0f, white);
		list[5] = std::make_shared<flip_normals>(
			std::make_shared<xy_rect>(0.0f, 555.0f, 0.0f, 555.0f, 555.0f, white));

		//list[6] =
		//	std::make_shared<translate>(
		//		std::make_shared<rotate_y>(
		//			std::make_shared<box>(vec3(0.0f, 0.0f, 0.0f), vec3(165.0f, 165.0f, 165.0f), white),
		//			-18.0f),
		//		vec3(130.0f, 0.0f, 65.0f));

		//list[7] =
		//	std::make_shared<translate>(
		//		std::make_shared<rotate_y>(
		//			std::make_shared<box>(vec3(0.0f, 0.0f, 0.0f), vec3(165.0f, 330.0f, 165.0f), white),
		//			15.0f),
		//		vec3(265.0f, 0.0f, 295.0f));

		auto white_isotropic = std::make_shared<isotropic>(white_tex);
		std::shared_ptr<texture> black_tex = std::make_shared<constant_texture>(vec3(0.0f, 0.0f, 0.0f));
		auto black_isotropic = std::make_shared<isotropic>(black_tex);

		list[6] =
		 	std::make_shared<constant_medium>(
		 		std::make_shared<translate>(
		 			std::make_shared<rotate_y>(
		 				std::make_shared<box>(vec3(0.0f, 0.0f, 0.0f), vec3(165.0f, 165.0f, 165.0f), white),
		 				-18.0f),
		 			vec3(130.0f, 0.0f, 65.0f)),
		 		0.01f,
		 		white_isotropic);

		list[7] =
		 	std::make_shared<constant_medium>(
		 		std::make_shared<translate>(
		 			std::make_shared<rotate_y>(
		 				std::make_shared<box>(vec3(0.0f, 0.0f, 0.0f), vec3(165.0f, 330.0f, 165.0f), white),
		 				15.0f),
		 			vec3(265.0f, 0.0f, 295.0f)),
		 		0.01f,
		 		black_isotropic);

		lookfrom = vec3(278.0f, 278.0f, -800.0f);
		lookat = vec3(278.0f, 278.0f, 0.0f);
		dist_to_focus = 10.0f;
		aperture = 0.0f;
		vfov = 40.0f;
	}

	hitable_list world(list, n);
	//vec3 lookfrom(-2, 2, 1); // before defocus
	//vec3 lookfrom(3, 3, 2); // defocus
	//vec3 lookat(0, 0, -1);

	camera cam(lookfrom, lookat, vec3(0.0f, 1.0f, 0.0f), vfov, float(nx) / float(ny), aperture, dist_to_focus, 0.0f, 1.0f);

	std::uniform_real_distribution<float> uniform;
	std::minstd_rand engine;

	std::vector<vec3> canvas(nx * ny);
	__int64 elapsedTrace = time_call([&]
	{
		// accelerated on this parallel
		_for(0, ny, 1, [&](int j)
		{
			// no obvious acceleration on this parallel, maybe even slower
			serial_for(0, nx, 1, [&](int i)
			{
				vec3 subPixels[subPixelCount];
				serial_for(0, subPixelCount, 1, [&](int s)
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
					subPixels[s] = color(r, &world, 0);
				});

				vec3 sum(0, 0, 0);
				for (auto& c : subPixels) // even slower with parallel_reduce
				{
					sum += c;
				}

				auto col = sum / static_cast<float>(subPixelCount);

				if (col.r != 0 && col.g != 0 && col.b != 0)
				{
					int aaa = 0;
				}

				// to gamma 2, and clamp
				col = vec3(std::min(sqrt(col.x), 1.0f), std::min(sqrt(col.y), 1.0f), std::min(sqrt(col.z), 1.0f));

				// save to canvas
				canvas[j * nx + i] = col;
			});
		});
	});

	std::ofstream out("1.ppm");
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

	std::cout.rdbuf(coutbuf); //reset to standard output again
	std::cout << "Trace: " << elapsedTrace << std::endl;
	std::cout << "Write: " << elapsedWrite << std::endl;

	out.close();

	// use ImageMagick to convert into easy to check format
#ifdef _WIN32
	system("convert 1.ppm 1.png");
	system("start 1.png");
#else
	system("/usr/local/bin/convert 1.ppm 1.png");
	system("open 1.png");
#endif

	return 0;
}