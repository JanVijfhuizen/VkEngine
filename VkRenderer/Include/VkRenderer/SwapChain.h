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

		struct Frame final
		{
			VkImage image;
			VkImageView imageView;
			VkFramebuffer frameBuffer;
			VkCommandBuffer commandBuffer;
		};

		VkSwapchainKHR swapChain;
		VkFormat format;
		VkExtent2D extent;
		std::vector<Frame> frames{};
		VkRenderPass renderPass;

		SwapChain();
		explicit SwapChain(VkRenderer& renderer);

		void Construct();
		void Cleanup();

		void SetRenderPass(VkRenderPass renderPass);

		[[nodiscard]] static SupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

	private:
		VkRenderer* _renderer;

		void CreateImages(VkDevice device);
		void CreateImageViews(VkDevice device);
		void CreateCommandBuffers(VkRenderer& renderer);

		void CreateFrameBuffers(VkDevice device, VkRenderPass renderPass);
		void CleanupFrameBuffers(VkDevice device);

		[[nodiscard]] static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		[[nodiscard]] static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		[[nodiscard]] VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
	};
}