#include "texture.h"

std::map<std::string, Texture> Texture::textures;

Texture::Texture(const std::string& path)
{
	data = (uint32_t*)stbi_load(path.c_str(), &width, &height, &channels, 4);
	if (width != height)
		return;
	for (auto x = 0; x < width; x++)
		for (auto y = 0; y < x; y++)
			std::swap(data[width * y + x], data[width * x + y]);
}

const uint32_t Texture::get_color(int x, int y)
{
    return *(data + x + y * width);
}

const std::vector<uint32_t> Texture::get_scaled_column(const int texture_coord, const int height)
{
	auto column = std::vector<uint32_t>(height);
	for (auto y = 0; y < height; y++) {
		auto _y = (y * width) / height;
		column[y] = get_color(_y, texture_coord);
	}
	return column;
}

const Texture& Texture::get_texture(std::string name)
{
	return textures[name];
}

const bool Texture::contains(const std::string& name)
{
	return textures.count(name) > 0;
}

void Texture::load_texture(const std::string& name, const std::string& path)
{
	auto txt = Texture(path);
	textures[name] = txt;
}
