#include "SDL/SDL.h"
#include "SDL/SDL_main.h"
#include <stdio.h>
#include "vec2.h"

#define SCALE 1

#define PI 3.14f
#define RADIANS(angle) angle * PI / 180.0f 
#define MAP_SIZE 8
#define WINDOW_WIDTH 1920/2
#define WINDOW_HEIGHT 1080/2
#define RENDER_WIDTH 640*SCALE
#define RENDER_HEIGHT 480*SCALE
#define FOV RADIANS(60)
#define RAY_STEP 0.01f
#define LAST_RAY_STEP 0.001f
#define DISTANCE_STEP 0.01f
#define BIN_RES 5
#define WALL_HEIGHT 64*2.6*SCALE
#define D_H 5
#define D_V 6

const int map[MAP_SIZE][MAP_SIZE] = { 
	{ 1, 1, 1, 1, 1, 1, 1, 1},
	{ 1, 0, 0, 0, 0, 0, 0, 1},
	{ 1, 0, 0, 0, 0, 0, 0, 1},
	{ 1, 1, 1, 5, 1, 1, 1, 1},
	{ 1, 0, 0, 0, 0, 0, 0, 1},
	{ 1, 0, 0, 0, 0, 0, 0, 1},
	{ 1, 0, 0, 0, 0, 0, 0, 1},
	{ 1, 1, 1, 1, 1, 1, 1, 1},
};

struct player
{
	vec2 pos;
	vec2 dir;
	vec2 plane;
	float angle = 0.0f;
	float rot_speed = 0.001f;
	float mov_speed = 0.04f;
	float radius = 0.5f;
};

void print_player_info(SDL_Window* window, player* player)
{
	char buffer[256];
	sprintf_s(buffer, "x = %f, y = %f, angle = %d\n", player->pos.x, player->pos.y, (int)(atan2f(player->dir.y, player->dir.x) * 180 / PI));
	SDL_SetWindowTitle(window, buffer);
}

void handle_mouse(SDL_Window* window, SDL_MouseMotionEvent* event, player* player)
{
	static int xpos = 0; 
	static int ypos = 0;
	xpos = event->xrel;
	ypos = event->yrel;	
	float angle = xpos * player->rot_speed;
	
	vec2 old_dir = player->dir;
	player->dir = vec2(
		player->dir.x * cosf(-angle) - player->dir.y * sinf(-angle),
		old_dir.x * sinf(-angle) + player->dir.y * cosf(-angle)
	);
	player->angle = atan2(player->dir.y, player->dir.x);
	vec2 old_plane = player->plane;
	player->plane = vec2(
		player->plane.x * cosf(-angle) - player->plane.y * sinf(-angle),
		old_plane.x * sinf(-angle) + player->plane.y * cosf(-angle)
	);
	//player->plane *=  SCALE;
	//print_player_info(window, player);
}

void handle_keyboard(SDL_KeyboardEvent* event, player* player, SDL_Window* window)
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
	player->pos += vec2(player->dir.x * player->mov_speed * vel_y, player->dir.y * player->mov_speed * vel_y);

	/*if (event->keysym.scancode == SDL_SCANCODE_W)
		player->pos += vec2(cosf(player->angle), -sinf(player->angle)) * player->mov_speed;
	if (event->keysym.scancode == SDL_SCANCODE_S)
		player->pos -= vec2(cosf(player->angle), -sinf(player->angle)) * player->mov_speed;
	if (event->keysym.scancode == SDL_SCANCODE_A)
		player->pos += vec2(cosf(player->angle - RADIANS(90)), -sinf(player->angle - RADIANS(90))) * player->mov_speed;
	if (event->keysym.scancode == SDL_SCANCODE_D)
		player->pos -= vec2(cosf(player->angle - RADIANS(90)), -sinf(player->angle - RADIANS(90))) * player->mov_speed;*/
	print_player_info(window, player);
}

void old_render(SDL_Renderer* renderer, player* player)
{
	for (int i = 0; i < RENDER_WIDTH; i++)
	{
		float raw_angle = ((float)i / RENDER_WIDTH) * FOV;
		float angle = player->angle + raw_angle - FOV / 2;
		vec2 new_pos = player->pos;
		vec2 old_ray;
		vec2 delta;
		while (map[(int)new_pos.x][(int)new_pos.y] == 0)
		{
			if ((int)new_pos.x > 0 && (int)new_pos.x < MAP_SIZE && (int)new_pos.y > 0 && (int)new_pos.y < MAP_SIZE)
			{
				old_ray = new_pos;
				new_pos += vec2(cosf(angle) * RAY_STEP, -sinf(angle) * RAY_STEP);
			}
			else break;
		}
		delta = new_pos - old_ray;
		new_pos = old_ray;
		while (map[(int)new_pos.x][(int)new_pos.y] == 0)
		{
			old_ray = new_pos;
			new_pos += vec2(cosf(angle) * LAST_RAY_STEP, -sinf(angle) * LAST_RAY_STEP);
		}
		
		float distance = vec2::distance(player->pos, old_ray);
		float correct_distance = distance * cosf(fabs(raw_angle - FOV / 2));
		int wall_height = (WALL_HEIGHT / correct_distance);
		SDL_Color render_color;
		if (map[(int)new_pos.x][(int)new_pos.y] == 1)
			render_color = { 50, 0, 100, 255 };
		if (map[(int)new_pos.x][(int)new_pos.y] == 2)
			render_color = { 100, 0, 255, 255 };
		if (map[(int)new_pos.x][(int)new_pos.y] == 3)
			render_color = { 200, 0, 255, 255 };
		if (map[(int)new_pos.x][(int)new_pos.y] == 4)
			render_color = { 200, 0, 100, 255 };
		if (map[(int)new_pos.x][(int)new_pos.y] == 5)
			render_color = { 50, 100, 210, 255 };
		SDL_SetRenderDrawColor(renderer, render_color.r, render_color.g, render_color.b, render_color.a);
		SDL_RenderDrawLine(renderer, i, RENDER_HEIGHT / 2 - wall_height, i, RENDER_HEIGHT / 2 + wall_height);

	}
}

void render(SDL_Renderer* renderer, player* player)
{
	for (int x = 0; x < RENDER_WIDTH; x++)
	{
		float camera_x = 2 * x / (float)RENDER_WIDTH - 1;
		vec2 ray_dir = vec2(
			player->dir.x + player->plane.x * camera_x, 
			player->dir.y + player->plane.y * camera_x
		);
		float mapX = (int)player->pos.x;
		float mapY = (int)player->pos.y;
		vec2 side_dist = vec2(0, 0);
		vec2 delta_dist = vec2(
			sqrtf(1.0f + (ray_dir.y * ray_dir.y) / (ray_dir.x * ray_dir.x)),
			sqrtf(1.0f + (ray_dir.x * ray_dir.x) / (ray_dir.y * ray_dir.y))
		);
		float distance = 0.0f;
		float step_x = 0, step_y = 0;
		float side = 0;
		if (ray_dir.x > 0)
		{
			step_x = 1;
			side_dist.x = (mapX + 1.0f - player->pos.x) * delta_dist.x;
		}
		else if (ray_dir.x < 0)
		{
			step_x = -1;
			side_dist.x = (player->pos.x - mapX) * delta_dist.x;
		}
		else {
			side_dist.x = 20000;
		}
		if (ray_dir.y > 0)
		{
			step_y = 1;
			side_dist.y = (mapY + 1.0f - player->pos.y) * delta_dist.y;
		}
		else if(ray_dir.y < 0)
		{
			step_y = -1;
			side_dist.y = (player->pos.y - mapY) * delta_dist.y;
		}else
		{
			side_dist.y = 20000;
		}
		int hit = 0;
		while (hit == 0)
		{
			if (side_dist.x < side_dist.y)
			{
				side_dist.x += delta_dist.x;
				mapX += step_x;
				side = 0.0f;
			} else {
				side_dist.y += delta_dist.y;
				mapY += step_y;
				side = 1.0f;
			}
			if (map[(int)mapX][(int)mapY] > 0) hit = 1;
		}
		if (side == 0) 
			distance = (mapX - player->pos.x + (1.0f - step_x) / 2.0f) / ray_dir.x;
		else           
			distance = (mapY - player->pos.y + (1.0f - step_y) / 2.0f) / ray_dir.y;
		if (map[(int)mapX][(int)mapY] == 5)
			distance += 0.4;
		int wall_height = (int)(WALL_HEIGHT / distance);
		float brightness;
		//brightness = (5.0f - vec2::distance(player->pos, vec2(mapX, mapY))) / 5.0f;
		brightness = (5.0f - distance) / 5.0f;
		if (brightness < 0) brightness = 0;
		if (brightness > 1) brightness = 1;
		//brightness = floor(brightness * 32) * 8;
		SDL_Color render_color = { brightness * 255, brightness * 255, brightness * 255, 255 };
		if (map[(int)mapX][(int)mapY] == 5) {
			render_color.r *= 0.2f;
			render_color.g *= 0.8f;
			render_color.b *= 0.9f;
			render_color.a *= 1.0f;
		}
		
		
		
		//if (map[mapX][mapY] == 5) render_color = { (Uint8)(brightness * 0.5f), (Uint8)(brightness * 0.2f), (Uint8)(brightness * 0.8f), 255 };
		SDL_SetRenderDrawColor(renderer, render_color.r, render_color.g, render_color.b, render_color.a);
		SDL_RenderDrawLine(renderer, x, RENDER_HEIGHT / 2 - wall_height, x, RENDER_HEIGHT / 2 + wall_height);
	}
}

void render_new(SDL_Renderer* renderer, player* player, SDL_Window* window) {
			
	for (int x = 0; x < RENDER_WIDTH; x++)
	{
		vec2 ray = player->pos;
		vec2 old = ray;

		float raw_angle = ((float)x / RENDER_WIDTH) * FOV;
		float angle = player->angle + raw_angle - FOV / 2;
		float err = 0;

		float stepX = cosf(angle);
		float stepY = sinf(angle);

		int rX = stepX > 0 ? 1 : -1;
		int rY = stepY > 0 ? 1 : -1;

		float distance = 0;
		do {
			if (!((int)ray.x > 0 && (int)ray.x < MAP_SIZE && (int)ray.y > 0 && (int)ray.y < MAP_SIZE))
				break;
			old = ray;
			ray.x += rX * 0.1f;
			ray.y += rY * 0.1f;
			distance += 0.5;
		} while (map[(int)ray.x][(int)ray.y] == 0);
		ray = old;
		do {
			if (!((int)ray.x > 0 && (int)ray.x < MAP_SIZE && (int)ray.y > 0 && (int)ray.y < MAP_SIZE))
				break;
			ray.x += stepX * 0.01f;
			ray.y += stepY*0.01f;
			distance += 0.01f;
		} while (map[(int)ray.x][(int)ray.y] == 0);

		/*char buffer[256];
		sprintf_s(buffer, "x = %f, y = %f, dis = %f", ray.x, ray.y, distance);
		SDL_SetWindowTitle(window, buffer);*/

		int wall_height = (int)(WALL_HEIGHT / distance);
		SDL_RenderDrawLine(renderer, x, RENDER_HEIGHT / 2 - wall_height, x, RENDER_HEIGHT / 2 + wall_height);
	}
}

int SDL_main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow("Raycasting", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC);
	player player;
	player.pos = vec2(5.5f, 3.5f);
	player.dir = vec2(-1, 0);
	player.plane = vec2(0.0f, 0.96f);
	SDL_RenderSetLogicalSize(renderer, RENDER_WIDTH, RENDER_HEIGHT);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_Texture* render_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, RENDER_WIDTH, RENDER_HEIGHT);
	while (true)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
			{
				handle_keyboard(&event.key, &player, window);
			} break;
			case SDL_MOUSEMOTION:
			{
				handle_mouse(window, &event.motion, &player);
			} break;
			default:
				break;
			}
		}
		SDL_SetRenderTarget(renderer, render_texture);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 100, 0, 100, 255);
		render(renderer, &player);
		SDL_SetRenderTarget(renderer, NULL);
		SDL_RenderCopyEx(renderer, render_texture, NULL, NULL, 0.0f, NULL, SDL_FLIP_NONE);
		SDL_RenderPresent(renderer);
		
	}
	return (0);
}