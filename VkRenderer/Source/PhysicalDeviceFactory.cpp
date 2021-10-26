#include "pch.h"
#include "PhysicalDeviceFactory.h"
#include "SwapChain.h"
#include "VkRenderer.h"

namespace vi
{
	PhysicalDeviceFactory::QueueFamilies::operator bool() const
	{
		for (const auto& family : values)
			if (family == UINT32_MAX)
				return false;
		return true;
	}

	PhysicalDeviceFactory::PhysicalDeviceFactory(VkRenderer& renderer, const Settings& settings)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(renderer._instance, &deviceCount, nullptr);
		assert(deviceCount);

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(renderer._instance, &deviceCount, devices.data());

		std::multimap<uint32_t, VkPhysicalDevice> candidates;

		for (const auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			const auto families = GetQueueFamilies(renderer._surface, device);
			if (!families)
				continue;

			if (!CheckDeviceExtensionSupport(device, renderer._settings->deviceExtensions))
				continue;

			const DeviceInfo deviceInfo
			{
				device,
				deviceProperties,
				deviceFeatures
			};

			if (!IsDeviceSuitable(renderer, settings, deviceInfo))
				continue;

			const uint32_t score = RateDevice(settings, deviceInfo);
			candidates.insert({ score, device });
		}
		assert(!candidates.empty());

		renderer._physicalDevice = candidates.rbegin()->second;
	}

	bool PhysicalDeviceFactory::IsDeviceSuitable(VkRenderer& renderer, const Settings& settings, const DeviceInfo& deviceInfo)
	{
		const auto swapChainSupport = SwapChain::QuerySwapChainSupport(renderer._surface, deviceInfo.device);
		if (!swapChainSupport)
			return false;

		auto& func = settings.deviceSuitableFunc;
		if (func)
			return func(deviceInfo);
		return true;
	}

	uint32_t PhysicalDeviceFactory::RateDevice(const Settings& settings, const DeviceInfo& deviceInfo)
	{
		auto& func = settings.deviceRatingFunc;
		if (func)
			return func(deviceInfo);

		uint32_t score = 0;
		auto& properties = deviceInfo.properties;

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000;
		score += properties.limits.maxImageDimension2D;

		return score;
	}

	bool PhysicalDeviceFactory::CheckDeviceExtensionSupport(const VkPhysicalDevice device,
		const std::vector<const char*>& extensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);
		return requiredExtensions.empty();
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
