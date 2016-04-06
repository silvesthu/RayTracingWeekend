// Test.cpp : Defines the entry point for the console application.
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

#include "gtest/gtest.h"

#include "../RayTracingWeekend/vec3.h"
#include "../RayTracingWeekend/ray.h"
#include "../RayTracingWeekend/material.h"

// just some simple test to avoid typo
TEST(vec3Test, size) 
{
	ASSERT_EQ(sizeof(vec3), 3 * sizeof(float));
}

TEST(vec3Test, dot) 
{
	auto d = dot(vec3(1, 1, 0), vec3(1, 1, 0));
	ASSERT_EQ(d, 2);
}

TEST(vec3Test, cross) 
{
	auto c = cross(vec3(1, 0, 0), vec3(0, 1, 0));
	ASSERT_EQ(c.x, 0);
	ASSERT_EQ(c.y, 0);
	ASSERT_EQ(c.z, 1);
}

TEST(RayTest, ctor)
{
	auto r = ray(vec3(1, 1, 1), vec3(2, 2, 2));
	ASSERT_EQ(r.point_at_parameter(3).x, 7);
	ASSERT_EQ(r.point_at_parameter(3).y, 7);
	ASSERT_EQ(r.point_at_parameter(3).z, 7);
}

TEST(ConcurrencyTest, Reduce)
{
	std::vector<int> i = { 1, 2, 30, 10, 4, 3, 1 };
	int int_min = INT_MIN;
	auto max_i = parallel_reduce(i.begin(), i.end(), int_min, [&](int result, int& next)
	{
		return std::max(result, next);
	});
	ASSERT_EQ(max_i, 30);
}

TEST(RandomTest, Random)
{
	std::uniform_real_distribution<float> distribution(0.0, 1.0);
	ASSERT_EQ(distribution.min(), 0.0f);
	ASSERT_EQ(distribution.max(), 1.0f);
}

TEST(VectorTest, Refract)
{
	vec3 refracted;
	auto result = refract(vec3(0, 0, 1), vec3(0, 0, -1), 1, refracted);
	ASSERT_TRUE(result);
	ASSERT_EQ(refracted.x, 0);
	ASSERT_EQ(refracted.y, 0);
	ASSERT_EQ(refracted.z, 1);
	result = refract(vec3(1, 1, 1), vec3(0, 0, -1), 1, refracted);
	ASSERT_TRUE(result);
	ASSERT_EQ(refracted.x, 1);
	ASSERT_EQ(refracted.y, 1);
	ASSERT_EQ(refracted.z, 1);
}

int _tmain(int argc, _TCHAR* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

