#include "input.h"

const uint8_t* input::keyboard = SDL_GetKeyboardState(NULL);
bool input::current[SDL_NUM_SCANCODES]{};
bool input::previous[SDL_NUM_SCANCODES]{};
int input::xpos = 0;
int input::ypos = 0;

void input::update()
{
	SDL_GetRelativeMouseState(&xpos, &ypos);
	for (auto i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		previous[i] = current[i];
		current[i] = keyboard[i];
	}
}

const bool input::get_key_down(int keydown)
{
	return current[keydown] && !previous[keydown];
}

const bool input::get_key(int keydown)
{
	return current[keydown] || previous[keydown];
}

const bool input::get_key_up(int keydown)
{
	return !current[keydown] && previous[keydown];
}

const vec2 input::get_mouse_axis()
{
	return vec2(xpos, ypos);
}
