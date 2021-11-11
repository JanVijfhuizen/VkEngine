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
			union
			{
				struct
				{
					VkImageView imageView;
					VkImageView depthImageView;
				};
				VkImageView imageViews[2];
			};

			VkImage image;
			VkImage depthImage;
			VkDeviceMemory depthImageMemory;

			VkFramebuffer frameBuffer;
			VkCommandBuffer commandBuffer;
		};

		struct Frame final
		{
			VkSemaphore imageAvailableSemaphore;
			VkSemaphore renderFinishedSemaphore;
			VkFence inFlightFence;
		};

		void Construct(VkRenderer& renderer);
		void Cleanup();

		void SetRenderPass(VkRenderPass renderPass);
		void GetNext(Image& outImage, Frame& outFrame);

		[[nodiscard]] VkResult Present();

		[[nodiscard]] VkRenderPass GetRenderPass() const;
		[[nodiscard]] VkFormat GetFormat() const;
		[[nodiscard]] VkFormat GetDepthBufferFormat() const;
		[[nodiscard]] VkExtent2D GetExtent() const;
		[[nodiscard]] uint32_t GetImageCount() const;
		[[nodiscard]] uint32_t GetCurrentImageIndex() const;

		[[nodiscard]] static SupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

	private:
		#define _MAX_FRAMES_IN_FLIGHT 2

		VkRenderer* _renderer;
		
		VkSwapchainKHR _swapChain;
		VkFormat _format;
		VkExtent2D _extent;
		std::vector<Frame> _frames{};
		std::vector<Image> _images{};
		std::vector<VkFence> _imagesInFlight{};
		VkRenderPass _renderPass;

		uint32_t _frameIndex = 0;
		uint32_t _imageIndex;

		void CreateImages();
		void CreateSyncObjects();

		void CreateBuffers();
		void CleanupBuffers();

		void WaitForImage();

		[[nodiscard]] static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		[[nodiscard]] static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		[[nodiscard]] VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
	};
}