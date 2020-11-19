#include <cmath>
#include <cstdio>
#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include "SDL/SDL_main.h"
#include "SDL/SDL.h"
#include "vec2.h"
#include "texture.h"
#include "input.h"



#define DEBUG_WALLS 0

const float scale = 1.0f;
const int width = 302 * scale, height = 160 * scale;

const int map_size = 10;
const float fov_scale = 1.0;
const int wall_height = height / fov_scale;


float z_map[width];

SDL_Renderer* renderer;
SDL_Window* window;

SDL_Texture* hud;

struct player_t
{
	vec2 position = vec2(6.2, 2.20);
	vec2 plane = vec2(0, 0.96 * fov_scale);
	vec2 direction = vec2(-1, 0);
	float angle = 90;
	float rot_speed = 0.0025f;
	float mov_speed = 0.04f;
	float radius = 0.5f;
} player;

struct ray_t
{
	int id;
	bool hit;
	bool side;
	float distance;
	int texture_x;
	bool door;
	bool door_side;
	bool invert_texture;
};

static uint32_t pack_rgba(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
	return (uint32_t)(r << 0 | g << 8 | b << 16 | a << 24);
}

static void draw_background(uint32_t data[])
{
	for (auto x = 0; x < width; x++)
	{
		for (auto y = 0; y < height; y++)
		{
			if (y < height / 2)
				data[x + y * width] = pack_rgba(46, 46, 46, 255);
			else
				data[x + y * width] = pack_rgba(110, 112, 111, 255);
		}
	}
}



int world_map[map_size][map_size] =
{
  {1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,1,5,1,1,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,0,1},
  {1,0,0,0,5,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1}
};

static inline void draw_pixel(uint32_t* data, int x, int y, uint32_t color)
{
	if(x >= 0 && x < width && y >= 0 && y < height)
		*(data + x + y * width) = color;
}

static void process_door(int dir, float& map_x, float& map_y, vec2 side_dist, vec2 ray_dir, vec2 delta_dist, int& side, int step_x, int step_y, ray_t& ray, bool& hit)
{
	//need to backup by the end
	auto last_x = map_x;
	auto last_y = map_y;

	auto distance = 0.0f;
	auto hit_point = 0.0f;

	//do an additional iteration to get next hit point, exactly like in cast_ray function but here we need the distance too
	if (side_dist.x < side_dist.y)
	{
		map_x += step_x;
		side = 0;
		distance = (map_x - player.position.x + (1 - step_x) / 2) / ray_dir.x;
	}
	else
	{
		map_y += step_y;
		side = 1;
		distance = (map_y - player.position.y + (1 - step_y) / 2) / ray_dir.y;
	}

	//getting a hit point from an additional iteration 
	if (side == 0)
		hit_point = player.position.y + distance * ray_dir.y;
	else
		hit_point = player.position.x + distance * ray_dir.x;
	hit_point -= floor((hit_point));

	// getting the offset relative to direction a ray is casted with
	auto offset = (dir == 0 ? ray_dir.x : ray_dir.y) > 0 ? 0.5 : 0.0;

	//check if ray actually hit a thin wall
	if (hit_point >= 0.0 + offset && hit_point < 0.5 + offset || (dir == 0 ? (last_x != map_x) : (last_y != map_y)))
	{
		side = dir;
		map_x = last_x + (dir == 0 ? step_x * 0.5f : 0);
		map_y = last_y + (dir == 1 ? step_y * 0.5f : 0);
		ray.door = true;
		hit = true;
	}
}

static ray_t cast_ray(const int x, const player_t& player)
{
	auto camera_x = 2 * x / (float)width - 1;
	auto ray_dir = player.direction + player.plane * camera_x;
	auto map_x = (float)(int)player.position.x;
	auto map_y = (float)(int)player.position.y;
	auto delta_dist = vec2(
		ray_dir.y == 0 ? 0 : ray_dir.x == 0 ? 1 : std::fabs(1.0f / ray_dir.x),
		ray_dir.x == 0 ? 0 : ray_dir.y == 0 ? 1 : std::fabs(1.0f / ray_dir.y)
	);
	auto side_dist = vec2();
	auto distance = 0.0f;
	auto step_x = 0;
	auto step_y = 0;
	auto hit = false;
	auto side = 0;
	if (ray_dir.x < 0)
	{
		step_x = -1;
		side_dist.x = (player.position.x - map_x) * delta_dist.x;
	}
	else if(ray_dir.x > 0)
	{
		step_x = 1;
		side_dist.x = (map_x + 1.0f - player.position.x) * delta_dist.x;
	}
	if (ray_dir.y < 0)
	{
		step_y = -1;
		side_dist.y = (player.position.y - map_y) * delta_dist.y;
	}
	else
	{
		step_y = 1;
		side_dist.y = (map_y + 1.0f - player.position.y) * delta_dist.y;
	}
	auto ray = ray_t();
	do
	{
		if (side_dist.x < side_dist.y)
		{
			side_dist.x += delta_dist.x;
			map_x += step_x;
			side = 0;

		}
		else
		{
			side_dist.y += delta_dist.y;
			map_y += step_y;
			side = 1;
		}
		if (world_map[(int)(map_x)][(int)(map_y)] == 5)
		{	
			if (side == 1)
			{
				process_door(1, map_x, map_y, side_dist, ray_dir, delta_dist, side, step_x, step_y, ray, hit);
				if ((ray_dir.y > 0) && ray.door)
					ray.invert_texture = true;
			}
			else
			{
				process_door(0, map_x, map_y, side_dist, ray_dir, delta_dist, side, step_x, step_y, ray, hit);
				if ((ray_dir.x > 0) && ray.door)
					ray.invert_texture = true;
			}
		}
		if (world_map[(int)(map_x)][(int)(map_y)] == 1)
		{
			if ((world_map[(int)(map_x)][(int)(map_y - 1)] == 5 || world_map[(int)(map_x)][(int)(map_y + 1)] == 5) && side == 1)
				ray.door_side = true;
			if ((world_map[(int)(map_x - 1)][(int)(map_y)] == 5 || world_map[(int)(map_x + 1)][(int)(map_y)] == 5) && side == 0)
				ray.door_side = true;
			hit = true;
		}
	} while (!hit);

	if (side == 0)
		distance = (map_x - player.position.x + (1 - step_x) / 2) / ray_dir.x;
	else
		distance = (map_y - player.position.y + (1 - step_y) / 2) / ray_dir.y;
	auto wall_x = 0.0f;
	if (side == 0)
		wall_x = player.position.y + distance * ray_dir.y;
	else
		wall_x = player.position.x + distance * ray_dir.x;
	wall_x -= std::floorf(wall_x);

	auto texture_x = (int)(wall_x * (float)64);
	if (side == 0 && ray_dir.x > 0)
		texture_x = 64 - texture_x - 1;
	if (side == 1 && ray_dir.y < 0)
		texture_x = 64 - texture_x - 1;

	ray.distance = distance;
	ray.hit = hit;
	ray.side = side;
	ray.id = world_map[(int)map_x][(int)map_y];
	ray.texture_x = texture_x;
	return ray;
}

static void render_textured_strip(uint32_t* data, const int x, const ray_t& ray, const player_t& player)
{
	auto texture = Texture::get_texture("wall");

	if (ray.door)
	{
		texture = Texture::get_texture("door");
	}
	else if (ray.door_side)
	{
		texture = Texture::get_texture("door_side");
	}
	auto line_height = (int)((wall_height - 1) / ray.distance);

	auto start = height / 2 - line_height / 2;
	auto end = height / 2 + line_height / 2;

	auto step = 1.0f * texture.get_height() / line_height;
	auto texture_pos = (start - end) * step;
	auto column = texture.get_scaled_column(ray.invert_texture ? (texture.get_width() - ray.texture_x - 1) : ray.texture_x, line_height);
	for (auto y = start; y < end; y++)
	{
		auto color = column[y - start];
		if (ray.side == 1 && !(ray.door || ray.door_side))
			color = (color >> 1) & 8355711;
		draw_pixel(data, x, y, color);
	}
}

static void load_textures()
{
	Texture::load_texture("wall", "textures/wall.bmp");
	Texture::load_texture("door", "textures/door.bmp");
	Texture::load_texture("door_side", "textures/door_side.bmp");
	auto surface = SDL_LoadBMP("textures/hud_base.bmp");

	hud = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
}

static void render_solid_strip(uint32_t* data, const int x, const ray_t& ray)
{
	auto line_height = (int)(wall_height / ray.distance);
	auto start = height / 2 - line_height / 2;
	auto end = height / 2 + line_height / 2;
	for (auto y = start; y < end; y++)
	{
		if (!ray.door)
		{
			draw_pixel(data, x, y, pack_rgba(200, 200, 200, 255));
		}
		else {
			draw_pixel(data, x, y, pack_rgba(200, 0, 200, 255));
		}
	}
}

static void draw_ui()
{
	SDL_RenderCopy(renderer, hud, nullptr, nullptr);
}

static void render(uint32_t *data, const player_t& player)
{
	draw_background(data);
	for (auto x = 0; x < width; x++)
	{
		auto ray = cast_ray(x, player);
		z_map[x] = ray.distance;
		if (ray.hit)
		{
			if(DEBUG_WALLS)
				render_solid_strip(data, x, ray);
			else
				render_textured_strip(data, x, ray, player);
		}
	}
}

static void update(player_t& player)
{
	player.angle = input::get_mouse_axis().x * player.rot_speed;
	vec2 old_dir = player.direction;
	player.direction = vec2(
		player.direction.x * cosf(-player.angle) - player.direction.y * sinf(-player.angle),
		old_dir.x * sinf(-player.angle) + player.direction.y * cosf(-player.angle)
	);
	vec2 old_plane = player.plane;
	player.plane = vec2(
		player.plane.x * cosf(-player.angle) - player.plane.y * sinf(-player.angle),
		old_plane.x * sinf(-player.angle) + player.plane.y * cosf(-player.angle)
	);

	auto speed = player.mov_speed;
	if (input::get_key(SDL_SCANCODE_W))
		player.position += vec2(player.direction.x * speed, player.direction.y * speed);
	if (input::get_key(SDL_SCANCODE_S))
		player.position += vec2(player.direction.x * -speed, player.direction.y * -speed);
	if (input::get_key(SDL_SCANCODE_A))
		player.position += vec2(player.direction.y * -speed, player.direction.x * speed);
	if (input::get_key(SDL_SCANCODE_D))
		player.position += vec2(player.direction.y * speed, player.direction.x * -speed);
}


int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Wolfenstein 3D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE );
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderSetLogicalSize(renderer, 320, 200);
	auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	auto interrupted = false;
	load_textures();

	while (!interrupted) {
		auto event = SDL_Event();
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
				{
					if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
						interrupted = true;
				} break;
			}
		}

		input::update();
		update(player);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		draw_ui();

		auto data = new uint32_t[width * height];
		render(data, player);
		SDL_UpdateTexture(texture, nullptr, &data[0], sizeof(uint32_t) * width);
		SDL_Rect dst = {0, 0, 320, 160};
		SDL_RenderCopy(renderer, texture, nullptr, &dst);
		SDL_RenderPresent(renderer);
		delete data;
		SDL_Delay(1000 / 60);
	}


	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

