#include "stdafx.h"
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "../RayTracingWeekend/material.h"

#include <ppl.h>
using namespace concurrency;

namespace CppTest
{		
	TEST_CLASS(_vec3)
	{
	public:		
		TEST_METHOD(_size)
		{
			Assert::AreEqual(sizeof(vec3), 3 * sizeof(float));
		}

		TEST_METHOD(_dot)
		{
			auto d = dot(vec3(1, 1, 0), vec3(1, 1, 0));
			Assert::AreEqual(d, 2.0);
		}

		TEST_METHOD(_cross)
		{
			auto c = cross(vec3(1, 0, 0), vec3(0, 1, 0));
			Assert::AreEqual(c.x, 0.0);
			Assert::AreEqual(c.y, 0.0);
			Assert::AreEqual(c.z, 1.0);
		}

		TEST_METHOD(_ctor)
		{
			auto r = ray(vec3(1, 1, 1), vec3(2, 2, 2), 0.0);
			Assert::AreEqual(r.point_at_parameter(3).x, 7.0);
			Assert::AreEqual(r.point_at_parameter(3).y, 7.0);
			Assert::AreEqual(r.point_at_parameter(3).z, 7.0);
		}
	};

	TEST_CLASS(_ppl)
	{
	public:
		TEST_METHOD(_reduce)
		{
			std::vector<int> i = { 1, 2, 30, 10, 4, 3, 1 };
			int int_min = INT_MIN;
			auto max_i = parallel_reduce(i.begin(), i.end(), int_min, [&](int result, int& next)
			{
				return std::max(result, next);
			});
			Assert::AreEqual(max_i, 30);
		}
	};

	TEST_CLASS(_random)
	{
	public:
		TEST_METHOD(_uniform)
		{
			std::uniform_real_distribution<double> distribution(0.0, 1.0);
			Assert::AreEqual(distribution.min(), 0.0);
			Assert::AreEqual(distribution.max(), 1.0);
		}
	};

	TEST_CLASS(_aabb)
	{
	public:
		TEST_METHOD(_hit)
		{
			Assert::IsTrue(
				aabb(vec3(2, 2, 2), vec3(4, 4, 4)).hit(
					ray(vec3(0,0,0), vec3(1,1,1), 0.0), 0, FLT_MAX));

			Assert::IsFalse(
				aabb(vec3(2, 2, 2), vec3(4, 4, 4)).hit(
					ray(vec3(0, 0, 0), -vec3(1, 1, 1), 0.0), 0, FLT_MAX));

			Assert::IsTrue(
				aabb(vec3(2, 2, 2), vec3(4, 4, 4)).hit(
					ray(vec3(3, 3, 3), vec3(0, 1, 0), 0.0), 0, FLT_MAX));

			Assert::IsTrue(
				aabb(vec3(2, 2, 2), vec3(4, 4, 4)).hit(
					ray(vec3(0, 3, 0), vec3(1, 0, 1), 0.0), 0, FLT_MAX));

			Assert::IsFalse(
				aabb(vec3(2, 2, 2), vec3(4, 4, 4)).hit(
					ray(vec3(0, 5, 0), vec3(1, 0, 1), 0.0), 0, FLT_MAX));
		}

		TEST_METHOD(_surrounding)
		{
			aabb box0(vec3(0, 0, 0), vec3(1, 1, 1));
			aabb box1(vec3(3, 3, 3), vec3(4, 4, 4));

			auto s = aabb::surrounding(box0, box1);

			Assert::AreEqual(s.min()[0], 0.0);
			Assert::AreEqual(s.min()[1], 0.0);
			Assert::AreEqual(s.min()[2], 0.0);

			Assert::AreEqual(s.max()[0], 4.0);
			Assert::AreEqual(s.max()[1], 4.0);
			Assert::AreEqual(s.max()[2], 4.0);
		}
	};
}