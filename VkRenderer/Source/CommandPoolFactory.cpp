#include "pch.h"
#include "CommandPoolFactory.h"
#include "PhysicalDeviceFactory.h"
#include "VkRenderer.h"

namespace vi
{
	CommandPoolFactory::CommandPoolFactory(VkRenderer& renderer)
	{
		const auto families = PhysicalDeviceFactory::GetQueueFamilies(renderer._surface, renderer._physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = families.graphics;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		const auto result = vkCreateCommandPool(renderer._device, &poolInfo, nullptr, &renderer._commandPool);
		assert(!result);
	}

	void CommandPoolFactory::Cleanup(VkRenderer& renderer)
	{
		vkDestroyCommandPool(renderer._device, renderer._commandPool, nullptr);
	}
}
