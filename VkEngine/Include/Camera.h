#pragma once
#include "ShaderSet.h"

struct Camera final
{
	struct Frame final
	{
		VkBuffer projectionBuffer;
		VkDeviceMemory projectionMemory;

		VkBuffer viewBuffer;
		VkDeviceMemory viewMemory;

		VkDescriptorSet descriptor;
	};

	class System final : public ShaderSet<Camera, Frame>
	{
	public:
		typedef Singleton<System> Instance;

		explicit System(uint32_t size);
	};
};
