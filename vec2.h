#pragma once
#include <math.h>

class vec2
{
public:
	vec2() : x(0.0f), y(0.0f) { };
	vec2(const float x, const float y);
	vec2 operator+ (const vec2& a) const;
	vec2& operator+= (const vec2& a);
	vec2 operator- (vec2 const& a) const;
	vec2 operator- ();
	vec2& operator-= (const vec2& a);
	vec2 operator* (vec2 const& a) const;
	vec2 operator/ (vec2 const& a) const;
	vec2 operator* (float s) const;
	vec2& operator*= (const vec2& a);
	vec2& operator*= (float s);
	bool operator==(const vec2& a) const;
	static vec2 lerp(vec2 start, vec2 end, float c);
	static vec2 get_normalized(vec2 v);
	static float get_magnitude(vec2 v);
	static vec2 floor(vec2 v);
	static float dot_product(const vec2 a, const vec2 b);
	static vec2 cross_product(const vec2 a, const vec2 b);
	static float distance(const vec2 a, const vec2 b);
	float x, y;
};
