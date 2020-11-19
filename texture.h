#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "stbi_image.h"
#include <map>

class Renderer;

class Texture
{
public:
	Texture() = default;
	Texture(const std::string& path);
	Texture(uint32_t* data, int w, int h) : data(data), width(w), height(h) {};

	const uint32_t get_color(int x, int y);

	const std::vector<uint32_t> get_scaled_column(const int texture_coord, const int height);

	const static Texture& get_texture(std::string name);

	const bool contains(const std::string& name);

	static void load_texture(const std::string& name, const std::string& path);

	const int get_width() { return width; };
	const int get_height() { return height; };

private:

	int width;
	int height;
	int channels;
	uint32_t* data;

	static std::map<std::string, Texture> textures;
};
