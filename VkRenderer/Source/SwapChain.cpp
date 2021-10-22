#include "pch.h"
#include "SwapChain.h"
#include "WindowSystem.h"
#include "PhysicalDeviceFactory.h"

namespace vi
{
	SwapChain::SupportDetails::operator bool() const
	{
		return !formats.empty() && !presentModes.empty();
	}

	uint32_t SwapChain::SupportDetails::GetRecommendedImageCount() const
	{
		uint32_t imageCount = capabilities.minImageCount + 1;

		const auto& maxImageCount = capabilities.maxImageCount;
		if (maxImageCount > 0 && imageCount > maxImageCount)
			imageCount = maxImageCount;

		return imageCount;
	}

	SwapChain::SwapChain()
	{
		_info = std::make_unique<Info>();
	}

	void SwapChain::Construct(const Info& info)
	{
		*_info = info;

		const SupportDetails support = QuerySwapChainSupport(info.surface, info.physicalDevice);
		const auto families = PhysicalDeviceFactory::GetQueueFamilies(info.surface, info.physicalDevice);

		const VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(support.formats);
		const VkPresentModeKHR presentMode = ChoosePresentMode(support.presentModes);

		_extent = ChooseExtent(info, support.capabilities);
		_format = surfaceFormat.format;

		const uint32_t imageCount = support.GetRecommendedImageCount();

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = info.surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = _format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = _extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // TODO: VK_IMAGE_USAGE_TRANSFER_DST_BIT for post processing.

		uint32_t queueFamilyIndices[] =
		{
			static_cast<uint32_t>(families.graphics),
			static_cast<uint32_t>(families.present)
		};

		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (families.graphics != families.present)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		createInfo.preTransform = support.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE; // Todo: store old swapchain.

		const auto result = vkCreateSwapchainKHR(info.device, &createInfo, nullptr, &_swapChain);
		assert(!result);

		CreateImages(imageCount);
		CreateImageViews();
	}

	void SwapChain::Cleanup()
	{
		for (const auto& imageView : _imageViews)
			vkDestroyImageView(_info->device, imageView, nullptr);

		_images.clear();
		_imageViews.clear();

		vkDestroySwapchainKHR(_info->device, _swapChain, nullptr);
	}

	SwapChain::SupportDetails SwapChain::QuerySwapChainSupport(const VkSurfaceKHR surface, const VkPhysicalDevice device)
	{
		SupportDetails details{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
				&presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkExtent2D SwapChain::GetExtent() const
	{
		return _extent;
	}

	VkSurfaceFormatKHR SwapChain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		return availableFormats.front();
	}

	VkPresentModeKHR SwapChain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availablePresentMode;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChain::ChooseExtent(const Info& info, const VkSurfaceCapabilitiesKHR& capabilities) const
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;

		const auto& windowSystem = info.windowSystem;
		const auto& resolution = windowSystem->GetVkInfo().resolution;

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(resolution.x),
			static_cast<uint32_t>(resolution.y)
		};

		const auto& minExtent = capabilities.minImageExtent;
		const auto& maxExtent = capabilities.maxImageExtent;

		const uint32_t actualWidth = std::max(minExtent.width, std::min(actualExtent.width, maxExtent.width));
		const uint32_t actualHeight = std::max(minExtent.height, std::min(actualExtent.height, maxExtent.height));
		actualExtent.width = actualWidth;
		actualExtent.height = actualHeight;

		return actualExtent;
	}

	void SwapChain::CreateImages(uint32_t count)
	{
		vkGetSwapchainImagesKHR(_info->device, _swapChain, &count, nullptr);
		_images.resize(count);
		vkGetSwapchainImagesKHR(_info->device, _swapChain, &count, _images.data());
	}

	void SwapChain::CreateImageViews()
	{
		const uint32_t count = _images.size();
		_imageViews.resize(count);

		for (uint32_t i = 0; i < count; ++i)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = _images[i];

			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = _format;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			const auto result = vkCreateImageView(_info->device, &createInfo, nullptr, &_imageViews[i]);
			assert(!result);
		}
	}
}
