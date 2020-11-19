#pragma once

#include "SDL/SDL.h"

#define HOR 0
#define VER 1
#define TEX_WIDTH 64
#define MAP_SIZE 10
#define RENDER_WIDTH 320
#define RENDER_HEIGHT 200
#define VIEWPORT_WIDTH 304
#define VIEWPORT_HEIGHT 152

extern int world_map[MAP_SIZE][MAP_SIZE] =
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

extern SDL_Window* window;
extern SDL_Renderer* renderer;