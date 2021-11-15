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

	void SwapChain::Construct(VkRenderer& renderer)
	{
		_renderer = &renderer;

		const SupportDetails support = QuerySwapChainSupport(_renderer->_surface, _renderer->_physicalDevice);
		const auto families = PhysicalDeviceFactory::GetQueueFamilies(_renderer->_surface, _renderer->_physicalDevice);

		const VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(support.formats);
		const VkPresentModeKHR presentMode = ChoosePresentMode(support.presentModes);

		_extent = ChooseExtent(support.capabilities);
		_format = surfaceFormat.format;

		const uint32_t imageCount = support.GetRecommendedImageCount();

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = _renderer->_surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = _format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = _extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

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

		auto& device = _renderer->_device;

		const auto result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &_swapChain);
		assert(!result);

		_images.resize(imageCount);
		_frames.resize(_MAX_FRAMES_IN_FLIGHT);
		_imagesInFlight.resize(_images.size(), VK_NULL_HANDLE);

		CreateImages();
		CreateSyncObjects();
	}

	void SwapChain::Cleanup()
	{
		for (auto& fence : _imagesInFlight)
			_renderer->WaitForFence(fence);
		_renderer->DeviceWaitIdle();

		auto& device = _renderer->_device;

		CleanupBuffers();

		for (const auto& image : _images)
			_renderer->DestroyImageView(image.imageView);
		_images.clear();

		for (const auto& frame : _frames)
		{
			_renderer->DestroySemaphore(frame.imageAvailableSemaphore);
			_renderer->DestroySemaphore(frame.renderFinishedSemaphore);
			_renderer->DestroyFence(frame.inFlightFence);
		}
		_frames.clear();
		_imagesInFlight.clear();

		vkDestroySwapchainKHR(device, _swapChain, nullptr);
	}

	void SwapChain::SetRenderPass(const VkRenderPass renderPass)
	{
		this->_renderPass = renderPass;
		CleanupBuffers();
		CreateBuffers();
	}

	void SwapChain::GetNext(Image& outImage, Frame& outFrame)
	{
		WaitForImage();
		outFrame = _frames[_frameIndex];
		outImage = _images[_imageIndex];
	}

	void SwapChain::WaitForImage()
	{
		auto& frame = _frames[_frameIndex];

		_renderer->WaitForFence(frame.inFlightFence);
		const auto result = vkAcquireNextImageKHR(_renderer->_device, _swapChain, UINT64_MAX, frame.imageAvailableSemaphore, VK_NULL_HANDLE, &_imageIndex);
		assert(!result);

		auto& imageInFlight = _imagesInFlight[_imageIndex];
		if (imageInFlight != VK_NULL_HANDLE)
			_renderer->WaitForFence(imageInFlight);
		imageInFlight = frame.inFlightFence;
	}

	VkResult SwapChain::Present()
	{
		auto& frame = _frames[_frameIndex];

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &frame.renderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &_swapChain;
		presentInfo.pImageIndices = &_imageIndex;

		const auto result = vkQueuePresentKHR(_renderer->_queues.present, &presentInfo);
		_frameIndex = (_frameIndex + 1) % _frames.size();

		return result;
	}

	VkRenderPass SwapChain::GetRenderPass() const
	{
		return _renderPass;
	}

	VkFormat SwapChain::GetFormat() const
	{
		return _format;
	}

	VkExtent2D SwapChain::GetExtent() const
	{
		return _extent;
	}

	uint32_t SwapChain::GetImageCount() const
	{
		return _images.size();
	}

	uint32_t SwapChain::GetCurrentImageIndex() const
	{
		return _imageIndex;
	}

	void SwapChain::CreateBuffers()
	{
		const uint32_t count = _images.size();

		std::vector<VkCommandBuffer> commandBuffers{};
		commandBuffers.resize(count);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = _renderer->_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		const auto result = vkAllocateCommandBuffers(_renderer->_device, &allocInfo, commandBuffers.data());
		assert(!result);

		const auto format = _renderer->GetDepthBufferFormat();
		const auto extent = GetExtent();

		for (uint32_t i = 0; i < count; ++i)
		{
			auto& image = _images[i];

			image.depthImage = _renderer->CreateImage({ extent.width, extent.height }, format, 
				VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
			image.depthImageMemory = _renderer->AllocateMemory(image.depthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			_renderer->BindMemory(image.depthImage, image.depthImageMemory);
			image.depthImageView = _renderer->CreateImageView(image.depthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT);

			auto cmdBuffer = _renderer->CreateCommandBuffer();
			const auto fence = _renderer->CreateFence();

			_renderer->BeginCommandBufferRecording(cmdBuffer);
			_renderer->TransitionImageLayout(image.depthImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			_renderer->EndCommandBufferRecording();
			_renderer->Submit(&cmdBuffer, 1, nullptr, nullptr, fence);
			_renderer->WaitForFence(fence);

			_renderer->DestroyCommandBuffer(cmdBuffer);
			_renderer->DestroyFence(fence);

			image.frameBuffer = _renderer->CreateFrameBuffer(image.imageViews, 2, _renderPass, _extent);
			image.commandBuffer = commandBuffers[i];
		}
	}

	void SwapChain::CleanupBuffers()
	{
		for (auto& image : _images)
		{
			_renderer->DestroyImageView(image.depthImageView);
			_renderer->DestroyImage(image.depthImage);
			_renderer->FreeMemory(image.depthImageMemory);

			_renderer->DestroyFrameBuffer(image.frameBuffer);
			_renderer->DestroyCommandBuffer(image.commandBuffer);
		}
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

		const auto& windowSystem = _renderer->_windowSystem;
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

	void SwapChain::CreateImages()
	{
		uint32_t count = _images.size();

		std::vector<VkImage> vkImages{};
		vkImages.resize(count);

		vkGetSwapchainImagesKHR(_renderer->_device, _swapChain, &count, vkImages.data());

		for (uint32_t i = 0; i < count; ++i)
		{
			auto& image = _images[i];
			image.image = vkImages[i];
			image.imageView = _renderer->CreateImageView(image.image, _format);
		}
	}

	void SwapChain::CreateSyncObjects()
	{
		for (auto& frame : _frames)
		{
			frame.imageAvailableSemaphore = _renderer->CreateSemaphore();
			frame.renderFinishedSemaphore = _renderer->CreateSemaphore();
			frame.inFlightFence = _renderer->CreateFence();
		}
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
