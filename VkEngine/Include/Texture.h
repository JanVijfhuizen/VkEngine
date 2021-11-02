#pragma once

struct Texture final
{
	glm::ivec2 resolution;
	uint32_t channels;
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
};