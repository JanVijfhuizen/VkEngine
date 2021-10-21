#include "pch.h"
#include "LogicalDeviceFactory.h"
#include "PhysicalDeviceFactory.h"

namespace vi
{
	LogicalDeviceFactory::LogicalDeviceFactory(const Info& info)
	{
		const auto queueFamilies = PhysicalDeviceFactory::GetQueueFamilies(info.surface, info.physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		const float queuePriority = 1.0f;

		for (const auto& family: queueFamilies.values)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = family;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledLayerCount = 0;
		if(DEBUG)
		{
			const auto& validationLayers = info.debugger.GetActiveValidationLayers();
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		const auto result = vkCreateDevice(info.physicalDevice, &createInfo, nullptr, &info.device);
		assert(!result);

		uint32_t i = 0;
		for (const auto& family : queueFamilies.values)
		{
			vkGetDeviceQueue(info.device, family, 0, &info.queues.values[i]);
			i++;
		}
	}
}
