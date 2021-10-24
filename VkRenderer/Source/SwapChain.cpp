#include "pch.h"
#include "SwapChain.h"
#include "PhysicalDeviceFactory.h"
#include "VkRenderer.h"
#include "WindowSystem.h"

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
	}

	SwapChain::SwapChain(VkRenderer& renderer) : _renderer(&renderer)
	{
		
	}

	void SwapChain::Construct()
	{
		const SupportDetails support = QuerySwapChainSupport(_renderer->surface, _renderer->physicalDevice);
		const auto families = PhysicalDeviceFactory::GetQueueFamilies(_renderer->surface, _renderer->physicalDevice);

		const VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(support.formats);
		const VkPresentModeKHR presentMode = ChoosePresentMode(support.presentModes);

		extent = ChooseExtent(support.capabilities);
		format = surfaceFormat.format;

		const uint32_t imageCount = support.GetRecommendedImageCount();

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = _renderer->surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
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

		auto& device = _renderer->device;

		const auto result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
		assert(!result);

		images.resize(imageCount);
		frames.resize(_MAX_FRAMES_IN_FLIGHT);

		CreateImages();
		CreateImageViews();
		CreateSyncObjects();
	}

	void SwapChain::Cleanup()
	{
		auto& device = _renderer->device;

		CleanupFrameBuffers();

		for (const auto& image : images)
			_renderer->DestroyImageView(image.imageView);
		images.clear();

		for (const auto& frame : frames)
		{	
			_renderer->DestroySemaphore(frame.imageAvailableSemaphore);
			_renderer->DestroySemaphore(frame.renderFinishedSemaphore);
			_renderer->DestroyFence(frame.inFlightFence);
		}
		frames.clear();

		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	void SwapChain::SetRenderPass(const VkRenderPass renderPass)
	{
		this->renderPass = renderPass;
		CleanupFrameBuffers();
		CreateFrameBuffers();
		CleanupCommandBuffers();
		CreateCommandBuffers();
	}

	void SwapChain::GetNext(Image*& outImage, Frame*& outFrame)
	{
		outFrame = &frames[_frameIndex];

		vkWaitForFences(_renderer->device, 1, &outFrame->inFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(_renderer->device, 1, &outFrame->inFlightFence);

		vkAcquireNextImageKHR(_renderer->device, swapChain, UINT64_MAX, outFrame->imageAvailableSemaphore, VK_NULL_HANDLE, &_imageIndex);
		outImage = &images[_imageIndex];
	}

	void SwapChain::Present(VkSubmitInfo& submitInfo)
	{
		auto& frame = frames[_frameIndex];

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &frame.renderFinishedSemaphore;

		const auto result = vkQueueSubmit(_renderer->queues.graphics, 1, &submitInfo, frame.inFlightFence);
		assert(!result);

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain;
		presentInfo.pImageIndices = &_imageIndex;

		vkQueuePresentKHR(_renderer->queues.present, &presentInfo);
		_frameIndex = (_frameIndex + 1) % frames.size();
	}

	void SwapChain::CreateFrameBuffers()
	{
		for (auto& image : images)
			image.frameBuffer = _renderer->CreateFrameBuffer(image.imageView, renderPass);
	}

	void SwapChain::CleanupFrameBuffers()
	{
		for (const auto& image : images)
			_renderer->DestroyFrameBuffer(image.frameBuffer);
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

	VkExtent2D SwapChain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;

		const auto& windowSystem = _renderer->windowSystem;
		const auto& resolution = windowSystem.GetVkInfo().resolution;

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

	void SwapChain::CreateImages()
	{
		uint32_t count = images.size();

		std::vector<VkImage> vkImages{};
		vkImages.resize(count);

		vkGetSwapchainImagesKHR(_renderer->device, swapChain, &count, vkImages.data());

		for (uint32_t i = 0; i < count; ++i)
			images[i].image = vkImages[i];
	}

	void SwapChain::CreateImageViews()
	{
		const uint32_t count = images.size();

		for (uint32_t i = 0; i < count; ++i)
		{
			auto& image = images[i];
			image.imageView = _renderer->CreateImageView(image.image, format);
		}
	}

	void SwapChain::CreateSyncObjects()
	{
		for (auto& frame : frames)
		{
			frame.imageAvailableSemaphore = _renderer->CreateSemaphore();
			frame.renderFinishedSemaphore = _renderer->CreateSemaphore();
			frame.inFlightFence = _renderer->CreateFence();
		}
	}

	void SwapChain::CreateCommandBuffers()
	{
		const uint32_t count = images.size();

		std::vector<VkCommandBuffer> commandBuffers{};
		commandBuffers.resize(count);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = _renderer->commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		const auto result = vkAllocateCommandBuffers(_renderer->device, &allocInfo, commandBuffers.data());
		assert(!result);

		for (uint32_t i = 0; i < count; ++i)
			images[i].commandBuffer = commandBuffers[i];
	}

	void SwapChain::CleanupCommandBuffers()
	{
		const uint32_t count = images.size();

		for (uint32_t i = 0; i < count; ++i)
			_renderer->DestroyCommandBuffer(images[i].commandBuffer);
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
}
