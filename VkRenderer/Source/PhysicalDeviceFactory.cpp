#include "pch.h"
#include "PhysicalDeviceFactory.h"

namespace vi
{
	PhysicalDeviceFactory::QueueFamilies::operator bool() const
	{
		for (const auto& family : values)
			if (family == UINT32_MAX)
				return false;
		return true;
	}

	PhysicalDeviceFactory::PhysicalDeviceFactory(const Info& info)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(info.instance, &deviceCount, nullptr);
		assert(deviceCount);

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(info.instance, &deviceCount, devices.data());

		std::multimap<uint32_t, VkPhysicalDevice> candidates;

		for (const auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			const auto families = GetQueueFamilies(info.surface, device);
			if (!families)
				continue;

			const DeviceInfo deviceInfo
			{
				deviceProperties,
				deviceFeatures
			};

			if (!IsDeviceSuitable(info, deviceInfo))
				continue;

			const uint32_t score = RateDevice(info, deviceInfo);
			candidates.insert({ score, device });
		}
		assert(!candidates.empty());

		info.physicalDevice = candidates.rbegin()->second;
	}

	bool PhysicalDeviceFactory::IsDeviceSuitable(const Info& info, const DeviceInfo& deviceInfo)
	{
		auto& func = info.settings.deviceSuitableFunc;
		if (func)
			return func(deviceInfo);
		return true;
	}

	uint32_t PhysicalDeviceFactory::RateDevice(const Info& info, const DeviceInfo& deviceInfo)
	{
		auto& func = info.settings.deviceRatingFunc;
		if (func)
			return func(deviceInfo);

		uint32_t score = 0;
		auto& properties = deviceInfo.properties;

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000;
		score += properties.limits.maxImageDimension2D;

		return score;
	}

	PhysicalDeviceFactory::QueueFamilies PhysicalDeviceFactory::GetQueueFamilies(const VkSurfaceKHR& surface, 
		const VkPhysicalDevice physicalDevice)
	{
		QueueFamilies families{};

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		uint32_t i = 0;
		for (const auto& queueFamily : queueFamilies) 
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				families.graphics = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (presentSupport)
				families.present = i;

			if (families)
				break;
			i++;
		}

		return families;
	}
}
