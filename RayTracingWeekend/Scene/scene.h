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
			std::make_shared<lambertian_color>(vec3(0.1f, 0.2f, 0.5f))));
		Add(std::make_shared<sphere>(vec3(0, -100.5f, -1), 100.0f,
			std::make_shared<lambertian_color>(vec3(0.8f, 0.8f, 0.0f))));
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

class cornell_box : public scene
{
public:
	cornell_box(float aspect) : scene()
	{
		// Cornell box
		std::shared_ptr<texture> red_tex = std::make_shared<constant_texture>(vec3(0.65f, 0.05f, 0.05f));
		auto red = std::make_shared<lambertian>(red_tex);
		std::shared_ptr<texture> white_tex = std::make_shared<constant_texture>(vec3(0.73f, 0.73f, 0.73f));
		auto white = std::make_shared<lambertian>(white_tex);
		std::shared_ptr<texture> green_tex = std::make_shared<constant_texture>(vec3(0.12f, 0.45f, 0.15f));
		auto green = std::make_shared<lambertian>(green_tex);
		std::shared_ptr<texture> light_tex = std::make_shared<constant_texture>(vec3(7.0f, 7.0f, 7.0f));
		auto light = std::make_shared<diffuse_light>(light_tex);

		std::vector<std::shared_ptr<hitable>> list;

		list.push_back(
			std::make_shared<xz_rect>(113.0f, 443.0f, 127.0f, 432.0f, 554.0f, light));

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