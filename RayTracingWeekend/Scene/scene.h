#pragma once

#include "../hitable_list.h"
#include "../camera.h"

enum class RenderType
{
	Shaded,
	Normal,
};

enum class BackgroundType
{
	Black,
	Gradient,
};

class scene
{
public:
	scene() {}
	virtual ~scene() {}

	void Add(std::shared_ptr<hitable> h) { world.list.push_back(h); }

	const hitable_list& GetWorld() const { return world; };
	RenderType GetRenderType() const { return render_type; }
	BackgroundType GetBackgroundType() const { return background_type; }

	camera& GetCamera() { return cam; };

protected:
	hitable_list world;
	camera cam;

	RenderType render_type = RenderType::Shaded;
	BackgroundType background_type = BackgroundType::Gradient;
};

class light_sample : public scene
{
public:
	light_sample(float aspect) : scene()
	{
		std::shared_ptr<texture> pertext = std::make_shared<noise_texture>(4.0f);
		std::shared_ptr<texture> four = std::make_shared<constant_texture>(vec3(4, 4, 4)); // color with scale

		std::vector<std::shared_ptr<hitable>> list;

		list.push_back(std::make_shared<sphere>(vec3(0, -1000, 0), 1000.0f,
			std::make_shared<lambertian>(pertext)));
		list.push_back(std::make_shared<sphere>(vec3(0, 2, 0), 2.0f,
			std::make_shared<lambertian>(pertext)));
		list.push_back(std::make_shared<sphere>(vec3(0, 7, 0), 2.0f,
			std::make_shared<diffuse_light>(four)));
		list.push_back(std::make_shared<xy_rect>(3.0f, 5.0f, 1.0f, 3.0f, -2.0f,
			std::make_shared<diffuse_light>(four)));

		auto lookfrom = vec3(24, 5, 5);
		auto lookat = vec3(0, 3, 0);
		auto dist_to_focus = (lookfrom - lookat).length();
		auto aperture = 0.2f;
		auto vfov = 20.0f;

		this->world = hitable_list(list);
		this->cam = camera(lookfrom, lookat, vec3(0.0f, 1.0f, 0.0f), vfov, aspect, aperture, dist_to_focus, 0.0f, 1.0f);
	}
};

class dielectric_scene : public scene
{
public:
	dielectric_scene(float aspect) : scene()
	{
		Add(std::make_shared<sphere>(vec3(0, 0, -1), 0.5f,
			std::make_shared<lambertian>(std::make_shared<constant_texture>(vec3(0.1f, 0.2f, 0.5f)))));
		Add(std::make_shared<sphere>(vec3(0, -100.5f, -1), 100.0f,
			std::make_shared<lambertian>(std::make_shared<constant_texture>(vec3(0.8f, 0.8f, 0.0f)))));
		Add(std::make_shared<sphere>(vec3(1, 0, -1), 0.5f,
			std::make_shared<metal>(vec3(0.8f, 0.6f, 0.2f), 0.0f)));
		Add(std::make_shared<sphere>(vec3(-1, 0, -1), 0.5f,
			std::make_shared<dielectric>(1.5f)));
		Add(std::make_shared<sphere>(vec3(-1, 0, -1), -0.45f,
			std::make_shared<dielectric>(1.5f)));

		auto lookfrom = vec3(0,0,0);
		auto lookat = vec3(0,0,-1);
		auto dist_to_focus = 10.0f;
		auto aperture = 0.0f;
		auto vfov = 120.0f;

		this->cam = camera(lookfrom, lookat, vec3(0.0f, 1.0f, 0.0f), vfov, aspect, aperture, dist_to_focus, 0.0f, 1.0f);
	}
};

class random_balls_scene : public scene
{
public:
	random_balls_scene(float aspect) : scene()
	{
		std::uniform_real_distribution<float> uniform;
		std::minstd_rand engine;

		// The ground
		std::shared_ptr<texture> groundAlbedo = std::make_shared<constant_texture>(vec3(0.5f, 0.5f, 0.5f));
		Add(std::make_shared<sphere>(vec3(0, -1000, 0), 1000.0f, std::make_shared<lambertian>(groundAlbedo)));

		// Small ones
		int i = 1;
		for (int a = -11; a < 11; a++)
			for (int b = -11; b < 11; b++)
			{
				float choose_mat = uniform(engine);
				vec3 center(a + 0.9f * uniform(engine), 0.2f, b + 0.9f * uniform(engine));
				if ((center - vec3(4.0f, 0.2f, 0.0f)).length() > 0.9f)
				{
					if (choose_mat < 0.8f) // diffuse
					{
						vec3 color;
						color.r = uniform(engine) * uniform(engine);
						color.g = uniform(engine) * uniform(engine);
						color.b = uniform(engine) * uniform(engine);
						std::shared_ptr<texture> albedo = std::make_shared<constant_texture>(color);
						bool moving = true;
						if (moving)
						{
							std::shared_ptr<moving_sphere> s = std::make_shared<moving_sphere>(center, 0.2f, std::make_shared<lambertian>(albedo));
							movement_linear m;
							m.center1 = center + vec3(0.0f, 0.5f * uniform(engine), 0.0f);
							m.time0 = 0.0f;
							m.time1 = 1.0f;
							s->set_movement(m);
							Add(s);
						}
						else
							Add(std::make_shared<sphere>(center, 0.2f, std::make_shared<lambertian>(albedo)));
					}
					else
					{
						if (choose_mat < 0.95) // metal
						{
							vec3 color;
							color.r = 0.5f * (1 + uniform(engine));
							color.g = 0.5f * (1 + uniform(engine));
							color.b = 0.5f * (1 + uniform(engine));
							float fuzz = 0.5f * uniform(engine);
							Add(std::make_shared<sphere>(center, 0.2f, std::make_shared<metal>(color, fuzz)));
						}
						else // glass
						{
							vec3 color;
							float ref_idx = 1.5f;
							Add(std::make_shared<sphere>(center, 0.2f, std::make_shared<dielectric>(ref_idx)));
						}
					}
				}
			}

		// Big ones
		Add(std::make_shared<sphere>(vec3(0,1,0), 1.0f, std::make_shared<dielectric>(1.5f)));
		Add(std::make_shared<sphere>(vec3(-4,1,0), 1.0f, std::make_shared<lambertian>(std::make_shared<constant_texture>(vec3(0.4f, 0.2f, 0.1f)))));
		Add(std::make_shared<sphere>(vec3(4,1,0), 1.0f, std::make_shared<metal>(vec3(0.7f, 0.6f, 0.5f), 0.0f)));

		// Camera
		auto lookfrom = vec3(13, 2, 3);
		auto lookat = vec3(0, 0, 0);
		auto dist_to_focus = 10.0f;
		auto aperture = 0.0f;
		auto vfov = 20.0f;
		this->cam = camera(lookfrom, lookat, vec3(0.0f, 1.0f, 0.0f), vfov, aspect, aperture, dist_to_focus, 0.0f, 1.0f);
	}
};

class cornell_box_scene : public scene
{
public:
	cornell_box_scene(float aspect) : scene()
	{
		// Cornell box
		std::shared_ptr<texture> red_tex = std::make_shared<constant_texture>(vec3(0.65f, 0.05f, 0.05f));
		auto red = std::make_shared<lambertian>(red_tex);
		std::shared_ptr<texture> white_tex = std::make_shared<constant_texture>(vec3(0.73f, 0.73f, 0.73f));
		auto white = std::make_shared<lambertian>(white_tex);
		std::shared_ptr<texture> green_tex = std::make_shared<constant_texture>(vec3(0.12f, 0.45f, 0.15f));
		auto green = std::make_shared<lambertian>(green_tex);
		std::shared_ptr<texture> light_tex = std::make_shared<constant_texture>(vec3(15.0f, 15.0f, 15.0f));
		auto light = std::make_shared<diffuse_light>(light_tex);

		std::vector<std::shared_ptr<hitable>> list;

		list.push_back(
			std::make_shared<xz_rect>(213.0f, 343.0f, 227.0f, 332.0f, 554.0f, light));

		list.push_back(
			std::make_shared<flip_normals>(
				std::make_shared<yz_rect>(0.0f, 555.0f, 0.0f, 555.0f, 555.0f, green)));
		list.push_back(
			std::make_shared<yz_rect>(0.0f, 555.0f, 0.0f, 555.0f, 0.0f, red));

		list.push_back(
			std::make_shared<flip_normals>(
				std::make_shared<xz_rect>(0.0f, 555.0f, 0.0f, 555.0f, 555.0f, white)));
		list.push_back(
			std::make_shared<xz_rect>(0.0f, 555.0f, 0.0f, 555.0f, 0.0f, white));
		list.push_back(
			std::make_shared<flip_normals>(
				std::make_shared<xy_rect>(0.0f, 555.0f, 0.0f, 555.0f, 555.0f, white)));

		list.push_back(
			std::make_shared<translate>(
				std::make_shared<rotate_y>(
					std::make_shared<box>(vec3(0.0f, 0.0f, 0.0f), vec3(165.0f, 165.0f, 165.0f), white),
					-18.0f),
				vec3(130.0f, 0.0f, 65.0f)));

		list.push_back(
			std::make_shared<translate>(
				std::make_shared<rotate_y>(
					std::make_shared<box>(vec3(0.0f, 0.0f, 0.0f), vec3(165.0f, 330.0f, 165.0f), white),
					15.0f),
				vec3(265.0f, 0.0f, 295.0f)));

		auto lookfrom = vec3(278.0f, 278.0f, -800.0f);
		auto lookat = vec3(278.0f, 278.0f, 0.0f);
		auto dist_to_focus = 10.0f;
		auto aperture = 0.0f;
		auto vfov = 40.0f;

		this->world = hitable_list(list);
		this->cam = camera(lookfrom, lookat, vec3(0.0f, 1.0f, 0.0f), vfov, aspect, aperture, dist_to_focus, 0.0f, 1.0f);
		this->background_type = BackgroundType::Black;
	}
};