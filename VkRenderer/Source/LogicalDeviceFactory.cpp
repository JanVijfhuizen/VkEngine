#include "pch.h"
#include "LogicalDeviceFactory.h"
#include "PhysicalDeviceFactory.h"
#include "VkRenderer.h"

namespace vi
{
	LogicalDeviceFactory::LogicalDeviceFactory(VkRenderer& renderer)
	{
		const auto queueFamilies = PhysicalDeviceFactory::GetQueueFamilies(renderer._surface, renderer._physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		const float queuePriority = 1.0f;

		std::set<uint32_t> familyIndexes{};

		for (const auto& family : queueFamilies.values)
		{
			if (familyIndexes.find(family) != familyIndexes.end())
				continue;
			familyIndexes.insert(family);

			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = family;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		const auto& deviceExtensions = renderer._settings->deviceExtensions;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		createInfo.enabledLayerCount = 0;
		if(DEBUG)
		{
			const auto& validationLayers = renderer._debugger.GetValidationLayers();
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		const auto result = vkCreateDevice(renderer._physicalDevice, &createInfo, nullptr, &renderer._device);
		assert(!result);

		uint32_t i = 0;
		for (const auto& family : queueFamilies.values)
		{
			vkGetDeviceQueue(renderer._device, family, 0, &renderer._queues.values[i]);
			i++;
		}
	}

	void LogicalDeviceFactory::Cleanup(VkRenderer& renderer)
	{
		vkDestroyDevice(renderer._device, nullptr);
	}
}
