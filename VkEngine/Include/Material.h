#pragma once

struct Material final
{
	struct Frame final
	{
		VkDescriptorSet descriptorSet;
		VkSampler diffuseSampler;
	};

	struct Texture* diffuseTexture = nullptr;
};
