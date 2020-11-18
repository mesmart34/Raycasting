#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "stbi_image.h"

struct texture_t
{
	texture_t() = default;
	texture_t(const std::string& path);
	texture_t(uint32_t* data, int w, int h) : data(data), width(w), height(h) {};

	const uint32_t get_color(int x, int y);

	const std::vector<uint32_t> get_scaled_column(const int texture_coord, const int height);

	int width;
	int height;
	int channels;
	uint32_t* data;
};
