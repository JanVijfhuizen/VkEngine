#pragma once

namespace vi
{
	class VkRenderer;

	class SwapChain final
	{
	public:
		struct SupportDetails final
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats{};
			std::vector<VkPresentModeKHR> presentModes{};

			[[nodiscard]] explicit operator bool() const;
			[[nodiscard]] uint32_t GetRecommendedImageCount() const;
		};

		struct Image final
		{
			VkImage image;
			VkImageView imageView;
			VkFramebuffer frameBuffer;
		};

		struct Frame final
		{
			VkCommandBuffer commandBuffer;
			VkSemaphore imageAvailableSemaphore;
			VkSemaphore renderFinishedSemaphore;
		};

		VkSwapchainKHR swapChain;
		VkFormat format;
		VkExtent2D extent;
		std::vector<Image> images{};
		std::vector<Frame> frames{};
		VkRenderPass renderPass;

		SwapChain();
		explicit SwapChain(VkRenderer& renderer);

		void Construct();
		void Cleanup();

		void SetRenderPass(VkRenderPass renderPass);
		void GetNext(Image*& outImage, Frame*& outFrame);

		void Present(VkSubmitInfo& submitInfo);

		[[nodiscard]] static SupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

	private:
		VkRenderer* _renderer;
		uint32_t _frameIndex = 0;
		uint32_t _imageIndex;

		void CreateImages();
		void CreateImageViews();
		void CreateSemaphores();

		void CreateCommandBuffers();
		void CleanupCommandBuffers();

		void CreateFrameBuffers();
		void CleanupFrameBuffers();

		[[nodiscard]] static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		[[nodiscard]] static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		[[nodiscard]] VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
	};
}