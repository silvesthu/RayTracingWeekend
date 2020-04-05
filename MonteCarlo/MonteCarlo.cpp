﻿#define _USE_MATH_DEFINES
#include <iostream>
#include <random>
#include <cmath>

#include "../RayTracingWeekend/vec3.h"
#include "../RayTracingWeekend/material.h"

void MonteCarlo_Estimate_PI()
{
	// See https://www.openfoam.com/documentation/guides/latest/api/Rand48_8H_source.html
	// for implementation of drand48 with std::linear_congruential_engine (underlying type of std::minstd_rand)

	std::uniform_real_distribution<float> uniform;
	std::minstd_rand engine;

	int N = 1000;
	int inside_circle = 0;
	for (int i = 0; i < N; i++)
	{
		float x = 2 * uniform(engine) - 1;
		float y = 2 * uniform(engine) - 1;
		
		if (x * x + y * y < 1)
			inside_circle++;
	}

	std::cout << "Estimate of PI = " << 4 * float(inside_circle) / N << "\n";
}

void MonteCarlo_EstimatePI_Forever()
{
	std::uniform_real_distribution<float> uniform;
	std::minstd_rand engine;

	int runs = 0;
	int inside_circle = 0;
	while (true)
	{
		runs++;
		
		float x = 2 * uniform(engine) - 1;
		float y = 2 * uniform(engine) - 1;

		if (x * x + y * y < 1)
			inside_circle++;
		
		if (runs % 100000 == 0)
			std::cout << "Estimate of PI = " << 4 * float(inside_circle) / runs << "\n";
	}
}

void MonteCarlo_EstimatePI_Stratified()
{
	std::uniform_real_distribution<float> uniform;
	std::minstd_rand engine;

	int inside_circle = 0;
	int inside_circle_stratified = 0;
	int sqrt_N = 10000;
	for (int i = 0; i < sqrt_N; i++)
	{
		for (int j = 0; j < sqrt_N; j++)
		{
			// Random
			float x = 2 * uniform(engine) - 1;
			float y = 2 * uniform(engine) - 1;

			if (x * x + y * y < 1)
				inside_circle++;

			// Jitter from uniform grid
			x = 2 * ((i + uniform(engine)) / sqrt_N) - 1;
			y = 2 * ((j + uniform(engine)) / sqrt_N) - 1;

			if (x * x + y * y < 1)
				inside_circle_stratified++;
		}
	}

	std::cout << "Regular		Estimate of Pi = " <<
		4 * float(inside_circle) / (sqrt_N * sqrt_N) << "\n";
	std::cout << "Stratified	Estimate of Pi = " <<
		4 * float(inside_circle_stratified) / (sqrt_N * sqrt_N) << "\n";
}

void MonteCarloIntegration_Uniform()
{
	std::uniform_real_distribution<float> uniform;
	std::minstd_rand engine;

	// integrate(x**2, (x, 0, 2))
	// with uniform distribution

	int inside_circle = 0;
	int N = 1000000;
	float sum = 0.0f;
	for (int i = 0; i < N; i++)
	{
		float x = 2 * uniform(engine);
		sum += x * x;
	}
	std::cout << "I = " << 2 * sum / N << "\n";

	// N uniform distributed thin bars with width = 1 / N, height = x * x
}

void MonteCarloIntegration_PDF1()
{
	std::uniform_real_distribution<float> uniform;
	std::minstd_rand engine;

	auto pdf = [](float x) { return 0.5f * x; };

	int N = 1000000;
	float sum = 0.0f;
	for (int i = 0; i < N; i++)
	{
		// pdf: 0.5*x
		// cdf: y = 0.25*x^2 by integrate(0.5*x)
		// inverse cdf: x = sqrt(4*y) = 2*sqrt(y) by solve(Eq(y, integrate(0.5*x)), x)
		float x = sqrt(4 * uniform(engine));
		sum += x * x / pdf(x);
	}

	std::cout << "I = " << sum / N << "\n";
}

void MonteCarloIntegration_PDF2()
{
	std::uniform_real_distribution<float> uniform;
	std::minstd_rand engine;

	auto pdf = [](float x) { return 3.0f * x * x / 8.0f; };

	int N = 1000000;
	float sum = 0.0f;
	for (int i = 0; i < N; i++)
	{
		// pdf: 3*x*x/8
		// cdf: y = x^3/8 by integrate(3*x*x/8)
		// inverse cdf: x = 2*y^(1/3)  by solve(Eq(y, integrate(3*x*x/8)), x)
		float x = pow(8.0f * uniform(engine), 1.0f / 3.0f);
		sum += x * x / pdf(x);
	}

	std::cout << "I = " << sum / N << "\n";
}

void MonteCarloIntegration_Sphere()
{
	auto random_on_unit_sphere = []()
	{
		static std::uniform_real_distribution<float> uniform;
		static std::minstd_rand engine;

		vec3 p = { 0, 0, 0 };
		do {
			vec3 random_vector(uniform(engine), uniform(engine), uniform(engine));
			p = 2.0f * random_vector - vec3(1, 1, 1); // -1 ~ 1 box
		} while (dot(p, p) >= 1.0f); // unit sphere
		return normalize(p);
	};

	auto pdf = [](const vec3& p) { return 1.0f / (4.0f * (float)M_PI); };

	int N = 1000000;
	float sum = 0.0f;
	for (int i = 0; i < N; i++)
	{
		vec3 d = random_on_unit_sphere();
		float cosine_suqared = d.z * d.z;
		sum += cosine_suqared / pdf(d);
	}

	std::cout << "I = " << sum / N << "\n";

	// integrate cos(theta)^2 over sphere
	// = integrate(integrate(cos(theta)**2*sin(theta), (theta, 0, pi)), (phi, 0, pi*2))
	// = 4*pi/3	
	// https://www.khanacademy.org/math/multivariable-calculus/integrating-multivariable-functions/triple-integrals-a/a/triple-integrals-in-spherical-coordinates
}

int main()
{
	// MonteCarlo_Estimate_PI();
	// MonteCarlo_EstimatePI_Forever();
	// MonteCarlo_EstimatePI_Stratified();

	// MonteCarloIntegration_Uniform();

	// https://en.wikipedia.org/wiki/Inverse_transform_sampling#Proof_of_correctness
	// Probability of result distribution F^-1(U)
	// = Pr(F^-1(U) <= x)	// Apply F(x) on both side
	// = Pr(U <= F(x))		// Pr(U <= y) = y as U is in uniform distribution
	// = F(x)				// The CDF
	// => F^-1(U) is in distribution of the PDF

	// MonteCarloIntegration_PDF1();
	// MonteCarloIntegration_PDF2();

	// MonteCarloIntegration_Sphere();

	return 0;
}