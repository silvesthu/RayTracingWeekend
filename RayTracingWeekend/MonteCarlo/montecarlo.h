#pragma once

template <typename XGetter, typename PDF>
void MonteCarlo1D(int N, XGetter xGetter,PDF pdf)
{
	std::uniform_real_distribution<float> uniform;
	std::minstd_rand engine;

	float sum = 0;
	for (int i = 0; i < N; i++)
	{
		float x = xGetter(uniform(engine));
		sum += (x * x) / pdf(x); // weight
	}

	auto result = sum / N;

	std::cout << __FUNCTION__ << std::endl;
	std::cout << "  I = " << result << std::endl;
	auto groundTruth = 8.0f / 3.0f;
	std::cout << "  Error = " << groundTruth - result << " from GroundTruth = " << groundTruth << std::endl;
}

// from http://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/monte-carlo-methods-mathematical-foundations/inverse-transform-sampling-method
template <int N, int SECTION_COUNT, typename PDF>
void MonteCarlo1DDiscreteSample(PDF pdf_func, float minBound = 0.0f, float maxBound = 1.0f)
{
	std::uniform_real_distribution<float> uniform;
	std::minstd_rand engine;

	float section = (maxBound - minBound) / SECTION_COUNT;

	// Calculate CDF
	float cdf[SECTION_COUNT + 1];
	cdf[0] = 0; // This ensure CDF starts 0 (which corrsponds with the extra constant when integral PDF to CDF)
	for (int i = 1; i < SECTION_COUNT; i++)
	{
		float x = minBound + i * section;
		float pdf = pdf_func(x);
		float pdf_x = pdf * section;
		cdf[i] = cdf[i - 1] + pdf_x;
	}
	cdf[SECTION_COUNT] = 1; // Be careful invalid PDF may still cause CDF goes to value other than 1, which is incorrect.

	// Take samples and put into appropriate bin
	float sum = 0;
	int bins[SECTION_COUNT] = { 0 };
	for (int i = 0; i < N; i++)
	{
		float r = uniform(engine);
		float *ptr = std::lower_bound(cdf, cdf + SECTION_COUNT + 1, r); // Prepare for inverse transform
		int off = std::max(0, (int)(ptr - cdf - 1));
		float t = (r - cdf[off]) / (cdf[off + 1] - cdf[off]); // Use similar triangle to avoid real inverse
		float x = (off + t) / (float)(SECTION_COUNT);
		float result = minBound + (maxBound - minBound) * x;
		bins[(int)(result / section)]++;
	}

	// Output bins
	for (int i = 0; i < SECTION_COUNT; i++)
	{
		//std::cout << (minBound + section * i) << "," << bins[i] << std::endl;
	}
}

void MonteCarlo()
{
	const int N = 100;

	// Analytically
	{
		// p(r) = 0.5f
		// P(r) = 0.5f * r <- integral of p(r)
		// e = P^-1(random) = 2 * r
		MonteCarlo1D(N, [](auto r) { return 2 * r; }, [](auto x) { return 0.5f; });

		// p(r) = r / 2
		// P(r) = 1/4 * r^2 <- integral of p(r)
		// e = P^-1(random) = sqrt(4 * random)
		MonteCarlo1D(N, [](auto r) { return sqrt(4 * r); }, [](auto x) { return x / 2.0f; });

		// p(r) = (3/8) * x^2
		// P(r) = (1/8) * x^3 <- integral of p(r)
		// e = P^-1(random) = pow(8x, 1/3)
		MonteCarlo1D(N, [](auto r) { return std::powf(8 * r, 1.0f / 3.0f); }, [](auto x) { return 3.0f * x * x / 8.0f; });
	}

	// Discrete
	{
		const int SampleCount = 100000;
		const int SectionCount = 1000;

		// p(r) = sin(x * 2 * pi) + 1 <- PDF
		// P(r) = integral(p(r))<- CDF
		MonteCarlo1DDiscreteSample<SampleCount, SectionCount>([](auto r)
		{
			return float(sin(r * M_PI * 2) + 1);
		});
	}

	// Spherical
	{
		auto normalized_random_in_unit_sphere = []()
		{
			static std::uniform_real_distribution<float> uniform;
			static std::minstd_rand engine;

			vec3 p = { 0, 0, 0 };
			do {
				auto random_vector = vec3(uniform(engine), uniform(engine), uniform(engine));
				p = 2.0f * random_vector - vec3(1, 1, 1); // -1 ~ 1 box
			} while (dot(p, p) >= 1.0f); // unit sphere
			return normalize(p);
		};

		auto pdf = [](const vec3& p) { return 1 / (4 * (float)M_PI); };

		const int N = 1000000;
		float sum = 0;

		for (int i = 0; i < N; i++)
		{
			vec3 d = normalized_random_in_unit_sphere();
			float cosine_squared = d.z * d.z;
			sum += cosine_squared / pdf(d);
		}
		std::cout << __FUNCTION__ << std::endl;
		std::cout << "  I = " << sum / N << std::endl;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Test scenes

//vec3 lookfrom(12, 2, 3);
//vec3 lookat(0, 0.5f, 0);
//float dist_to_focus = (lookfrom - lookat).length();
//float aperture = 0.2f;
//float vfov = 20.0f;

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