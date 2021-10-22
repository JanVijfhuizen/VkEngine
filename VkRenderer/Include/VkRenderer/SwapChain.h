#pragma once

namespace vi
{
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

		struct Info final
		{
			VkPhysicalDevice physicalDevice;
			VkSurfaceKHR surface;
			VkDevice device;
			class WindowSystem* windowSystem;
		};

		SwapChain();
		void Construct(const Info& info);
		void Cleanup();

		[[nodiscard]] static SupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);
		[[nodiscard]] VkFormat GetFormat() const;
		[[nodiscard]] VkExtent2D GetExtent() const;

	private:
		std::unique_ptr<Info> _info;

		VkSwapchainKHR _swapChain;
		VkFormat _format;
		VkExtent2D _extent;

		std::vector<VkImage> _images{};
		std::vector<VkImageView> _imageViews{};

		[[nodiscard]] static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		[[nodiscard]] static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		[[nodiscard]] VkExtent2D ChooseExtent(const Info& info, const VkSurfaceCapabilitiesKHR& capabilities) const;

		void CreateImages(uint32_t count);
		void CreateImageViews();
	};
}