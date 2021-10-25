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
		};

		struct Frame final
		{
			VkSemaphore imageAvailableSemaphore;
			VkSemaphore renderFinishedSemaphore;
			VkFence inFlightFence;
		};

		struct Buffers final
		{
			VkFramebuffer frameBuffer;
			VkCommandBuffer commandBuffer;
		};

		VkSwapchainKHR swapChain;
		VkFormat format;
		VkExtent2D extent;
		std::vector<Image> images{};
		std::vector<Frame> frames{};
		std::vector<Buffers> buffers{};
		std::vector<VkFence> imagesInFlight{};
		VkRenderPass renderPass;

		explicit SwapChain(VkRenderer& renderer);
		~SwapChain();

		void SetRenderPass(VkRenderPass renderPass);
		void GetNext(Buffers*& outBuffers, Frame*& outFrame);

		void WaitForImage();
		[[nodiscard]] VkResult Present();

		[[nodiscard]] static SupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);

	private:
		#define _MAX_FRAMES_IN_FLIGHT 2

		VkRenderer* _renderer;
		uint32_t _frameIndex = 0;
		uint32_t _bufferIndex = 0;
		uint32_t _imageIndex;

		void CreateImages();
		void CreateSyncObjects();

		void CreateBuffers();
		void CleanupBuffers();

		[[nodiscard]] static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		[[nodiscard]] static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		[[nodiscard]] VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
	};
}