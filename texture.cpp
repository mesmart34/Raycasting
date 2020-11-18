#include "texture.h"

texture_t::texture_t(const std::string& path)
{
	data = (uint32_t*)stbi_load(path.c_str(), &width, &height, &channels, 4);
}

const uint32_t texture_t::get_color(int x, int y)
{
    return *(data + x + y * width);
}

const std::vector<uint32_t> texture_t::get_scaled_column(const int texture_coord, const int height)
{
	std::vector<uint32_t> column(height);
	for (size_t y = 0; y < height; y++) {
		auto _y = (y * width) / height;
		column[y] = get_color(texture_coord, _y);
	}
	return column;
}
