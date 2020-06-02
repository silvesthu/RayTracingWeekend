#define _USE_MATH_DEFINES
#include <iostream>
#include <random>
#include <cmath>
#include <iomanip>

#include "../RayTracingWeekend/vec3.h"
#include "../RayTracingWeekend/material.h"

void MonteCarlo_Estimate_PI()
{
	// See https://www.openfoam.com/documentation/guides/latest/api/Rand48_8H_source.html
	// for implementation of drand48 with std::linear_congruential_engine (underlying type of std::minstd_rand)

	std::uniform_real_distribution<double> uniform;
	std::minstd_rand engine;

	int N = 1000;
	int inside_circle = 0;
	for (int i = 0; i < N; i++)
	{
		double x = 2 * uniform(engine) - 1;
		double y = 2 * uniform(engine) - 1;
		
		if (x * x + y * y < 1)
			inside_circle++;
	}

	std::cout << "Estimate of PI = " << 4 * inside_circle / N << "\n";
}

void MonteCarlo_EstimatePI_Forever()
{
	std::uniform_real_distribution<double> uniform;
	std::minstd_rand engine;

	int runs = 0;
	int inside_circle = 0;
	while (true)
	{
		runs++;
		
		double x = 2 * uniform(engine) - 1;
		double y = 2 * uniform(engine) - 1;

		if (x * x + y * y < 1)
			inside_circle++;
		
		if (runs % 100000 == 0)
			std::cout << "Estimate of PI = " << 4 * inside_circle / runs << "\n";
	}
}

void MonteCarlo_EstimatePI_Stratified()
{
	std::uniform_real_distribution<double> uniform;
	std::minstd_rand engine;

	int inside_circle = 0;
	int inside_circle_stratified = 0;
	int sqrt_N = 10000;
	for (int i = 0; i < sqrt_N; i++)
	{
		for (int j = 0; j < sqrt_N; j++)
		{
			// Random
			double x = 2 * uniform(engine) - 1;
			double y = 2 * uniform(engine) - 1;

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
	std::uniform_real_distribution<double> uniform;
	std::minstd_rand engine;

	// integrate(x**2, (x, 0, 2))
	// with uniform distribution

	int inside_circle = 0;
	int N = 1000000;
	double sum = 0.0;
	for (int i = 0; i < N; i++)
	{
		double x = 2 * uniform(engine);
		sum += x * x;
	}
	std::cout << "I = " << 2 * sum / N << "\n";

	// N uniform distributed thin bars with width = 1 / N, height = x * x
}

void MonteCarloIntegration_PDF1()
{
	std::uniform_real_distribution<double> uniform;
	std::minstd_rand engine;

	auto pdf = [](double x) { return 0.5 * x; };

	int N = 1000000;
	double sum = 0.0;
	for (int i = 0; i < N; i++)
	{
		// pdf: 0.5*x
		// cdf: y = 0.25*x^2 by integrate(0.5*x)
		// inverse cdf: x = sqrt(4*y) = 2*sqrt(y) by solve(Eq(y, integrate(0.5*x)), x)
		double x = sqrt(4 * uniform(engine));
		sum += x * x / pdf(x);
	}

	std::cout << "I = " << sum / N << "\n";
}

void MonteCarloIntegration_PDF2()
{
	std::uniform_real_distribution<double> uniform;
	std::minstd_rand engine;

	auto pdf = [](double x) { return 3.0 * x * x / 8.0; };

	int N = 1000000;
	double sum = 0.0;
	for (int i = 0; i < N; i++)
	{
		// pdf: 3*x*x/8
		// cdf: y = x^3/8 by integrate(3*x*x/8)
		// inverse cdf: x = 2*y^(1/3)  by solve(Eq(y, integrate(3*x*x/8)), x)
		double x = pow(8.0 * uniform(engine), 1.0 / 3.0);
		sum += x * x / pdf(x);
	}

	std::cout << "I = " << sum / N << "\n";
}

void MonteCarloIntegration_Sphere()
{
	auto random_on_unit_sphere = []()
	{
		static std::uniform_real_distribution<double> uniform;
		static std::minstd_rand engine;

		vec3 p = { 0, 0, 0 };
		do {
			vec3 random_vector(uniform(engine), uniform(engine), uniform(engine));
			p = 2.0 * random_vector - vec3(1, 1, 1); // -1 ~ 1 box
		} while (dot(p, p) >= 1.0); // unit sphere
		return normalize(p);
	};

	auto pdf = [](const vec3& p) { return 1.0 / (4.0 * M_PI); };

	int N = 1000000;
	double sum = 0.0;
	for (int i = 0; i < N; i++)
	{
		vec3 d = random_on_unit_sphere();
		double cosine_suqared = d.z * d.z;
		sum += cosine_suqared / pdf(d);
	}

	std::cout << "I = " << sum / N << "\n";

	// integrate cos(theta)^2 over sphere
	// = integrate(integrate(cos(theta)**2*sin(theta), (theta, 0, pi)), (phi, 0, pi*2))
	// = 4*pi/3	
	// https://www.khanacademy.org/math/multivariable-calculus/integrating-multivariable-functions/triple-integrals-a/a/triple-integrals-in-spherical-coordinates
}

void Generate_Random_Directions()
{
	// book3.chapter7

	// to generate random directions

	// assume 
	//   z-axis = surface normal
	//   theta = angle from the normal

	// p(direction) = f(theta)

	// integration over spherical coordinates
	//   dA = sin(theta) * dtheta * dphi

	// PDF
	//   p(phi) = 1/2pi <= the circle
	//   p(theta) = 2pi * f(theta) * sin(theta) <= p(direction)dA

	// Distributions r_1, r_2 - see book3.chapter3 about inverse CDF
	//   r_1 = integrate(1/2pi, dt, 0, phi) = phi/2pi
	//     phi = 2pi * r_1
	//   r_2 = integrate(2pi * f(t) *sin(t), dt, 0, theta)
	//     for uniform density on sphere, p(direction) = f(theta) = 1/4pi
	//     theta = (1 - cos(theta)) / 2

	{
		static std::uniform_real_distribution<double> uniform;
		static std::minstd_rand engine;

		for (int i = 0; i < 200; i++)
		{
			// uniform distribution
			auto r1 = uniform(engine);
			auto r2 = uniform(engine);

			// x,y,z in distribution of theta, phi in sphere
			auto x = cos(2 * M_PI * r1) * 2 * sqrt(r2 * (1 - r2));
			auto y = sin(2 * M_PI * r1) * 2 * sqrt(r2 * (1 - r2));
			auto z = 1 - 2 * r2;

			// p(direction) = 1/4pi

			std::cout << x << " " << y << " " << z << '\n';
		}
	}

	std::cout << '\n';

	{
		static std::uniform_real_distribution<double> uniform;
		static std::minstd_rand engine;

		int N = 1000000;
		auto sum = 0.0;
		for (int i = 0; i < N; i++)
		{
			// uniform distribution
			auto r1 = uniform(engine);
			auto r2 = uniform(engine);

			// x,y,z in distribution of theta, phi in hemisphere
			auto x = cos(2 * (float)M_PI * r1) * 2 * sqrt(r2 * (1 - r2));
			auto y = sin(2 * (float)M_PI * r1) * 2 * sqrt(r2 * (1 - r2));
			auto z = 1 - r2;

			// cosine cubed
			// p(direction) = 1/2pi
			sum += z * z * z / (1.0 / (2.0 * (float)M_PI));
		}
		std::cout << std::fixed << std::setprecision(12);
		std::cout << "Pi/2     = " << (float)M_PI / 2 << '\n'; // Known solution by direct integration
		std::cout << "Estimate = " << sum / N << '\n';
	}

	std::cout << '\n';

	{
		int N = 1000000;

		auto sum = 0.0;
		for (int i = 0; i < N; i++)
		{
			static std::uniform_real_distribution<double> uniform;
			static std::minstd_rand engine;

			// uniform distribution
			auto r1 = uniform(engine);
			auto r2 = uniform(engine);

			// x,y,z in distribution of theta, phi in cosine distribution
			auto x = cos(2 * (float)M_PI * r1) * sqrt(r2);
			auto y = sin(2 * (float)M_PI * r1) * sqrt(r2);
			auto z = sqrt(1 - r2);

			// p(direction) = cos(theta)/pi
			sum += z * z * z / (z / (float)M_PI);
		}

		std::cout << std::fixed << std::setprecision(12);
		std::cout << "Pi/2     = " << (float)M_PI / 2 << '\n';
		std::cout << "Estimate = " << sum / N << '\n';
	}
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

	// Generate_Random_Directions();

	// for Plot
	{
		std::cout << "[";
		for (int i = 0; i < 1000; i++)
		{
			if (i != 0)
				std::cout << ", ";

			vec3 v;

			onb uvw;
			uvw.build_from_w(vec3(0, 0, 1));
			v = uvw.local(random_cosine_direction());

			// v = normalize(random_in_hemisphere(vec3(0,0,1)));

			std::cout << "[" << v.x << ", " << v.y << ", " << v.z << "]";
		}
		std::cout << "]";
	}

	return 0;
}