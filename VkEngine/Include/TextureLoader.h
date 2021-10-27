#pragma once

class TextureLoader final
{
public:
	static unsigned char* Load(const std::string& path, int32_t& width, int32_t& height, int32_t& numChannels);
	static void Free(unsigned char* pixels);
};
