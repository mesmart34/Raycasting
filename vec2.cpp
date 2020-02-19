#include "vec2.h"

static float Q_rsqrt(float number)
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *(long*)& y;                       // evil floating point bit level hacking
	i = 0x5f3759df - (i >> 1);               // what the fuck? 
	y = *(float*)& i;
	y = y * (threehalfs - (x2 * y * y));   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

vec2::vec2(const float x, const float y)
{
	this->x = x;
	this->y = y;
}


vec2 vec2::operator+(const vec2& a) const
{
	return vec2(this->x + a.x, this->y + a.y);
}

vec2& vec2::operator+=(const vec2& a)
{
	this->x += a.x;
	this->y += a.y;
	return *this;
}

vec2 vec2::operator-(vec2 const& a) const
{
	return vec2(this->x - a.x, this->y - a.y);
}

vec2 vec2::operator-()
{
	return  vec2(-x, -y);
}

vec2& vec2::operator-=(const vec2& a)
{
	this->x -= a.x;
	this->y -= a.y;
	return *this;
}

vec2 vec2::operator*(vec2 const& a) const
{
	return vec2(this->x * a.x, this->y * a.y);
}

vec2 vec2::operator/(vec2 const& a) const
{
	return vec2(this->x / a.x, this->y / a.y);
}

vec2 vec2::operator*(float s) const
{
	return vec2(this->x * s, this->y * s);
}

vec2& vec2::operator*=(const vec2& a)
{
	this->x *= a.x;
	this->y *= a.y;
	return *this;
}

vec2& vec2::operator*=(float s)
{
	this->x *= s;
	this->y *= s;
	return *this;
}

bool vec2::operator==(const vec2& a) const
{
	return fabs(this->x - a.x) < 0.0001f && fabs(this->y - a.y) < 0.0001f;
}

vec2 vec2::lerp(const vec2 start, const vec2 end, const float c)
{
	return start + (end - start) * c;
}

vec2 vec2::get_normalized(vec2 v)
{
	const auto magnitude = get_magnitude(v);
	return vec2(v.x / magnitude, v.y / magnitude);
}

float vec2::get_magnitude(vec2 v)
{
	return sqrtf(v.x * v.x + v.y * v.y);
}

vec2 vec2::floor(vec2 v)
{
	return vec2((int)v.x, (int)v.y);
}

float vec2::dot_product(const vec2 a, const vec2 b)
{
	return a.x * b.x - a.y * b.y;
}

vec2 vec2::cross_product(const vec2 a, const vec2 b)
{
	return vec2();
}

float vec2::distance(const vec2 a, const vec2 b)
{
	return 1.0f / Q_rsqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}
