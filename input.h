#pragma once

#include "SDL/SDL.h"
#include <cstdint>
#include "vec2.h"

class input
{
public:
	static void update();
	const static bool get_key_down(int keydown);
	const static bool get_key(int keydown);
	const static bool get_key_up(int keydown);
	const static vec2 get_mouse_axis();
private:
	input() = default;
	const static uint8_t* keyboard;
	static bool current[SDL_NUM_SCANCODES];
	static bool previous[SDL_NUM_SCANCODES];
	static int xpos;
	static int ypos;
};