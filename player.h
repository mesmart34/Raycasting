#pragma once

#include "vec2.h"

struct player_t
{
	vec2 position;
	vec2 plane;
	vec2 direction;
	float move_speed;
	float rotation_speed;
	float radius;
};
