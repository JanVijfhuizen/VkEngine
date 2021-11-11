#pragma once

struct DepthBuffer final
{
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory imageMemory;
};