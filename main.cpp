#include "SDL/SDL_main.h"
#include "SDL/SDL.h"
#include <cstdint>
#include"vec2.h"
#include <cmath>
#include <cstdio>
#include <map>
#include <vector>
#include <string>
#include "texture.h"
#include <chrono>

using namespace std::chrono_literals;

#define TEXTURED 1

const int width = 640 / 2, height = width / 16 * 9;
using namespace std;

const int map_size = 10;
const int wall_height = height;

constexpr std::chrono::nanoseconds timestep(16ms);


struct player_t
{
	vec2 position = vec2(6.2, 2.20);
	vec2 plane = vec2(0, 0.96);
	vec2 direction = vec2(-1, 0);
	float angle = 90;
	float rot_speed = 0.011f;
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
	int door_x, door_y;
	int hit_x, hit_y;
	float door_distance;
	int door_id;
};

vector<float> z_map(width);
std::map<string, texture_t> textures;

void handle_mouse(SDL_Window* window, SDL_MouseMotionEvent* event, player_t* player)
{
	static int xpos = 0;
	static int ypos = 0;
	xpos = event->xrel;
	ypos = event->yrel;
	player->angle = xpos * player->rot_speed;
	vec2 old_dir = player->direction;
	player->direction = vec2(
		player->direction.x * cosf(-player->angle) - player->direction.y * sinf(-player->angle),
		old_dir.x * sinf(-player->angle) + player->direction.y * cosf(-player->angle)
	);
	vec2 old_plane = player->plane;
	player->plane = vec2(
		player->plane.x * cosf(-player->angle) - player->plane.y * sinf(-player->angle),
		old_plane.x * sinf(-player->angle) + player->plane.y * cosf(-player->angle)
	);
}


void handle_keyboard(SDL_KeyboardEvent* event, player_t* player, SDL_Window* window)
{
	float vel_x = 0.0f, vel_y = 0.0f;
	if (event->keysym.scancode == SDL_SCANCODE_W)
		vel_y = 1.0f;
	if (event->keysym.scancode == SDL_SCANCODE_S)
		vel_y = -1.0f;
	if (event->keysym.scancode == SDL_SCANCODE_A)
		vel_x = -1.0f;
	if (event->keysym.scancode == SDL_SCANCODE_D)
		vel_x = 1.0f;
	player->position += vec2(player->direction.x * player->mov_speed * vel_y, player->direction.y * player->mov_speed * vel_y);
	//player->position += vec2(player->direction.y * player->mov_speed * vel_x, player->direction.x * player->mov_speed * vel_x);
}

static void load_textures()
{
	textures["wall"] = texture_t("textures/wall.bmp");
	textures["door_side"] = texture_t("textures/door_side.bmp");
	textures["door"] = texture_t("textures/door.bmp");
}

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
  {1,0,0,0,1,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,0,1},
  {1,0,0,0,1,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1}
};

static inline void draw_pixel(uint32_t* data, int x, int y, uint32_t color)
{
	if(x >= 0 && x < width && y >= 0 && y < height)
		*(data + x + y * width) = color;
}

static bool intersects(const vec2& a, const vec2& b, const vec2& c, const vec2& d)
{
	auto v1 = (d.x - c.x) * (a.y - c.y) - (c.y - c.y) * (a.x - c.x);
	auto v2 = (d.x - c.x) * (b.x - c.y) - (c.y - c.y) * (b.x - c.x);
	auto v3 = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	auto v4 = (b.x - a.x) * (d.y - a.y) - (b.y - a.y) * (d.x - a.x);
	return (v1 * v2 < 0) && (v3 * v4 < 0);
}

static ray_t track_ray(const int x, const player_t& player)
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

	while (!hit)
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
			auto last_x = map_x;
			auto last_y = map_y;
			auto last_side_dist = side_dist;
			auto last_delta_dist = delta_dist;
			if (side_dist.x < side_dist.y)
			{
				side_dist.x += delta_dist.x;
				map_x += step_x;
				side = 0;
				distance = (map_x - player.position.x + (1 - step_x) / 2) / ray_dir.x;

			}
			else
			{
				side_dist.y += delta_dist.y;
				map_y += step_y;
				side = 1;
				distance = (map_y - player.position.y + (1 - step_y) / 2) / ray_dir.y;
			}
			double wallX;
			if (side == 0) 
				wallX = player.position.y + distance * ray_dir.y;
			else           
				wallX = player.position.x + distance * ray_dir.x;
			wallX -= floor((wallX));
			//printf("%f\n", wallX);
			if (wallX >= 0.0 && wallX < 0.5f && last_x == map_x)
			{
				ray.door = true;
				map_x = last_x + step_x * 0.5f;
				map_y = last_y;
				delta_dist = last_delta_dist;
				side_dist = last_side_dist;
				side = 0;
				hit = true;
			}
			else {
				if (world_map[(int)(map_x)][(int)(map_y)] != 1)
				{
					hit = true;
					ray.door = true;
					map_x = last_x;
					map_x += step_x * 0.5f;
					map_y = last_y;
					delta_dist = last_delta_dist;
					side_dist = last_side_dist;
					
				}
			}
		}
		if (world_map[(int)(map_x)][(int)(map_y)] == 1)
		{
			if ((world_map[(int)(map_x)][(int)(map_y - 1)] == 5 || world_map[(int)(map_x)][(int)(map_y + 1)] == 5) && side == 1)
				ray.door_side = true;
			hit = true;
		}
	}

	if (side == 0)
	{
		distance = (map_x - player.position.x + (1 - step_x) / 2);
		distance /= ray_dir.x;
	}
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
	auto texture = textures["wall"];

	if (ray.door)
	{
		texture = textures["door"];
	}
	else if (ray.door_side)
	{
		texture = textures["door_side"];
	}
	auto line_height = (int)((wall_height - 1) / ray.distance);

	auto start = height / 2 - line_height / 2;
	auto end = height / 2 + line_height / 2;

	auto step = 1.0f * texture.height / line_height;
	auto texture_pos = (start - end - 0.001) * step;
	auto column = texture.get_scaled_column(ray.texture_x, line_height);
	for (auto y = start; y < end; y++)
	{
		auto color = column[y - start];
		if (ray.side == 1) 
			color = (color >> 1) & 8355711;
		draw_pixel(data, x, y, color);
	}
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

static void render(uint32_t *data, const player_t& player)
{
	draw_background(data);
	for (auto x = 0; x < width; x++)
	{
		auto ray = track_ray(x, player);
		z_map[x] = ray.distance;
		if (ray.hit)
		{
			if(TEXTURED)
				render_textured_strip(data, x, ray, player);
			else
				render_solid_strip(data, x, ray);
		}
	}
}



int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	auto window = SDL_CreateWindow("Wolfenstein 3D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE );
	auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderSetLogicalSize(renderer, width, height);
	auto texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	auto interrupted = false;
	load_textures();

	using clock = std::chrono::high_resolution_clock;

	std::chrono::nanoseconds lag(0ns);
	auto time_start = clock::now();
	bool quit_game = false;


	while (!interrupted) {
		auto delta_time = clock::now() - time_start;
		time_start = clock::now();
		lag += std::chrono::duration_cast<std::chrono::nanoseconds>(delta_time);

		auto event = SDL_Event();
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
					interrupted = true;
				handle_keyboard(&event.key, &player, window);
			} break;
			case SDL_MOUSEMOTION:
			{
				handle_mouse(window, &event.motion, &player);
			} break;
			}
		}

		// update game logic as lag permits
		while (lag >= timestep) {
			lag -= timestep;

			
			//update(&current_state); // update at a fixed rate each time
		}

		// calculate how close or far we are from the next timestep
		/*auto alpha = (float)lag.count() / timestep.count();
		auto interpolated_state = interpolate(current_state, previous_state, alpha);*/

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		auto data = new uint32_t[width * height];
		render(data, player);
		SDL_UpdateTexture(texture, nullptr, &data[0], sizeof(uint32_t) * width);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);
		delete data;
	}


	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}