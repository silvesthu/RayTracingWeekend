#pragma once

#include "../hittable_list.h"
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

	void Add(std::shared_ptr<hittable> h) { world.objects.push_back(h); }

	const hittable_list& GetWorld() const { return world; };
	std::shared_ptr<hittable_list> GetLights() const { return lights; }
	RenderType GetRenderType() const { return render_type; }
	BackgroundType GetBackgroundType() const { return background_type; }

	camera& GetCamera() { return cam; };

protected:
	hittable_list world;
	std::shared_ptr<hittable_list> lights = std::make_shared<hittable_list>();
	camera cam;

	RenderType render_type = RenderType::Shaded;
	BackgroundType background_type = BackgroundType::Gradient;
};

class light_sample : public scene
{
public:
	light_sample(double aspect) : scene()
	{
		std::shared_ptr<texture> pertext = std::make_shared<noise_texture>(4.0);
		std::shared_ptr<texture> four = std::make_shared<constant_texture>(vec3(4, 4, 4)); // color with scale

		std::vector<std::shared_ptr<hittable>> objects;

		objects.push_back(std::make_shared<sphere>(vec3(0, -1000, 0), 1000.0,
			std::make_shared<lambertian>(pertext)));
		objects.push_back(std::make_shared<sphere>(vec3(0, 2, 0), 2.0,
			std::make_shared<lambertian>(pertext)));
		objects.push_back(std::make_shared<sphere>(vec3(0, 7, 0), 2.0,
			std::make_shared<diffuse_light>(four)));
		objects.push_back(std::make_shared<xy_rect>(3.0, 5.0, 1.0, 3.0, -2.0,
			std::make_shared<diffuse_light>(four)));

		auto lookfrom = vec3(24, 5, 5);
		auto lookat = vec3(0, 3, 0);
		auto dist_to_focus = (lookfrom - lookat).length();
		auto aperture = 0.2f;
		auto vfov = 20.0;

		this->world = hittable_list(objects);
		this->cam = camera(lookfrom, lookat, vec3(0.0, 1.0, 0.0), vfov, aspect, aperture, dist_to_focus, 0.0, 1.0);
	}
};

class dielectric_scene : public scene
{
public:
	dielectric_scene(double aspect) : scene()
	{
		Add(std::make_shared<sphere>(vec3(0, 0, -1), 0.5f,
			std::make_shared<lambertian>(std::make_shared<constant_texture>(vec3(0.1f, 0.2f, 0.5f)))));
		Add(std::make_shared<sphere>(vec3(0, -100.5f, -1), 100.0,
			std::make_shared<lambertian>(std::make_shared<constant_texture>(vec3(0.8f, 0.8f, 0.0)))));
		Add(std::make_shared<sphere>(vec3(1, 0, -1), 0.5f,
			std::make_shared<metal>(vec3(0.8f, 0.6f, 0.2f), 0.0)));
		Add(std::make_shared<sphere>(vec3(-1, 0, -1), 0.5f,
			std::make_shared<dielectric>(1.5f)));
		Add(std::make_shared<sphere>(vec3(-1, 0, -1), -0.45f,
			std::make_shared<dielectric>(1.5f)));

		auto lookfrom = vec3(0,0,0);
		auto lookat = vec3(0,0,-1);
		auto dist_to_focus = 10.0;
		auto aperture = 0.0;
		auto vfov = 120.0;

		this->cam = camera(lookfrom, lookat, vec3(0.0, 1.0, 0.0), vfov, aspect, aperture, dist_to_focus, 0.0, 1.0);
	}
};

class random_balls_scene : public scene
{
public:
	random_balls_scene(double aspect) : scene()
	{
		std::uniform_real_distribution<double> uniform;
		std::minstd_rand engine;

		// The ground
		std::shared_ptr<texture> groundAlbedo = std::make_shared<constant_texture>(vec3(0.5f, 0.5f, 0.5f));
		Add(std::make_shared<sphere>(vec3(0, -1000, 0), 1000.0, std::make_shared<lambertian>(groundAlbedo)));

		// Small ones
		int i = 1;
		for (int a = -11; a < 11; a++)
			for (int b = -11; b < 11; b++)
			{
				double choose_mat = uniform(engine);
				vec3 center(a + 0.9f * uniform(engine), 0.2f, b + 0.9f * uniform(engine));
				if ((center - vec3(4.0, 0.2f, 0.0)).length() > 0.9f)
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
							m.center1 = center + vec3(0.0, 0.5f * uniform(engine), 0.0);
							m.time0 = 0.0;
							m.time1 = 1.0;
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
							double fuzz = 0.5f * uniform(engine);
							Add(std::make_shared<sphere>(center, 0.2f, std::make_shared<metal>(color, fuzz)));
						}
						else // glass
						{
							vec3 color;
							double ref_idx = 1.5f;
							Add(std::make_shared<sphere>(center, 0.2f, std::make_shared<dielectric>(ref_idx)));
						}
					}
				}
			}

		// Big ones
		Add(std::make_shared<sphere>(vec3(0,1,0), 1.0, std::make_shared<dielectric>(1.5f)));
		Add(std::make_shared<sphere>(vec3(-4,1,0), 1.0, std::make_shared<lambertian>(std::make_shared<constant_texture>(vec3(0.4f, 0.2f, 0.1f)))));
		Add(std::make_shared<sphere>(vec3(4,1,0), 1.0, std::make_shared<metal>(vec3(0.7f, 0.6f, 0.5f), 0.0)));

		// Camera
		auto lookfrom = vec3(13, 2, 3);
		auto lookat = vec3(0, 0, 0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.0;
		auto vfov = 20.0;
		this->cam = camera(lookfrom, lookat, vec3(0.0, 1.0, 0.0), vfov, aspect, aperture, dist_to_focus, 0.0, 1.0);
	}
};

class cornell_box_scene : public scene
{
public:
	cornell_box_scene(double aspect) : scene()
	{
		// Cornell box
		std::shared_ptr<texture> red_tex = std::make_shared<constant_texture>(vec3(0.65f, 0.05f, 0.05f));
		auto red = std::make_shared<lambertian>(red_tex);
		std::shared_ptr<texture> white_tex = std::make_shared<constant_texture>(vec3(0.73f, 0.73f, 0.73f));
		auto white = std::make_shared<lambertian>(white_tex);
		std::shared_ptr<texture> green_tex = std::make_shared<constant_texture>(vec3(0.12f, 0.45f, 0.15f));
		auto green = std::make_shared<lambertian>(green_tex);
		std::shared_ptr<texture> light_tex = std::make_shared<constant_texture>(vec3(15.0, 15.0, 15.0));
		auto light = std::make_shared<diffuse_light>(light_tex);

		std::vector<std::shared_ptr<hittable>> objects;

		objects.push_back(
			std::make_shared<xz_rect>(213.0, 343.0, 227.0, 332.0, 554.0, light));
		lights->objects.push_back(objects.back());

		objects.push_back(
			std::make_shared<flip_normals>(
				std::make_shared<yz_rect>(0.0, 555.0, 0.0, 555.0, 555.0, green)));
		objects.push_back(
			std::make_shared<yz_rect>(0.0, 555.0, 0.0, 555.0, 0.0, red));

		objects.push_back(
			std::make_shared<flip_normals>(
				std::make_shared<xz_rect>(0.0, 555.0, 0.0, 555.0, 555.0, white)));
		objects.push_back(
			std::make_shared<xz_rect>(0.0, 555.0, 0.0, 555.0, 0.0, white));
		objects.push_back(
			std::make_shared<flip_normals>(
				std::make_shared<xy_rect>(0.0, 555.0, 0.0, 555.0, 555.0, white)));

		objects.push_back(
			std::make_shared<translate>(
				std::make_shared<rotate_y>(
					std::make_shared<box>(vec3(0.0, 0.0, 0.0), vec3(165.0, 165.0, 165.0), white),
					-18.0),
				vec3(130.0, 0.0, 65.0)));

#if 1 // book3.chapter12 - glass sphere
 		objects.pop_back(); // pop short box

 		objects.push_back(
 			std::make_shared<sphere>(vec3(190, 90, 190), 90, std::make_shared<dielectric>(1.5)));
		lights->objects.push_back(objects.back());
#endif // book3.chapter12.4

		std::shared_ptr<material> tall_box_material = white;
#if 0 // book3.chapter12.2 - aluminum box
		std::shared_ptr<material> aluminum = std::make_shared<metal>(vec3(0.8, 0.85, 0.88), 0.0);
		tall_box_material = aluminum;
#endif // book3.chapter12.2
		
		objects.push_back(
			std::make_shared<translate>(
				std::make_shared<rotate_y>(
					std::make_shared<box>(vec3(0.0, 0.0, 0.0), vec3(165.0, 330.0, 165.0), tall_box_material),
					15.0),
				vec3(265.0, 0.0, 295.0)));

		auto lookfrom = vec3(278.0, 278.0, -800.0);
		auto lookat = vec3(278.0, 278.0, 0.0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.0;
		auto vfov = 40.0;

		this->world = hittable_list(objects);
		this->cam = camera(lookfrom, lookat, vec3(0.0, 1.0, 0.0), vfov, aspect, aperture, dist_to_focus, 0.0, 1.0);
		this->background_type = BackgroundType::Black;
	}
};