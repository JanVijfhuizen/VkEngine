#include "pch.h"
#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

unsigned char* TextureLoader::Load(const std::string& path, int32_t& width, int32_t& height, int32_t& numChannels)
{
	const auto pixels =  stbi_load(path.c_str(), &width, &height, &numChannels, STBI_rgb_alpha);
	assert(pixels);
	return pixels;
}

void TextureLoader::Free(unsigned char* pixels)
{
	stbi_image_free(pixels);
}
