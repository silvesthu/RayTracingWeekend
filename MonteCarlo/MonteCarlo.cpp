#define _USE_MATH_DEFINES
#include <iostream>
#include <random>
#include <cmath>

#include "../RayTracingWeekend/vec3.h"

template <typename XGetter, typename PDF>
void MonteCarlo1D(int N, XGetter xGetter, PDF pdf)
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
		float* ptr = std::lower_bound(cdf, cdf + SECTION_COUNT + 1, r); // Prepare for inverse transform
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

template <int N, typename PDF>
void MonteCarloSpherical(PDF pdf)
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

void MonteCarlo()
{
	// Analytically
	{
		const int N = 100;

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
		const int N = 1000000;

		// p(r) = 1 / (4 * pi) <- PDF
		MonteCarloSpherical<N>([](const vec3& p) { return 1 / (4 * (float)M_PI); });
	}
}

int main()
{
	MonteCarlo();
	return 0;
}