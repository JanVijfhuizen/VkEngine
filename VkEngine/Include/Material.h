#pragma once

struct Material final
{
	struct Frame final
	{
		VkSampler diffuseSampler;
	};

	std::vector<Frame> frames{};
	Texture* diffuseTexture;
};