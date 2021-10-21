#pragma once

namespace vi
{
	class PhysicalDeviceFactory final
	{
	public:
		struct Info;

		struct DeviceInfo final
		{
			const VkPhysicalDevice& device;
			const VkPhysicalDeviceProperties& properties;
			const VkPhysicalDeviceFeatures& features;
		};

		struct QueueFamilies final
		{
			union
			{
				struct
				{
					uint32_t graphics;
					uint32_t present;
				};

				uint32_t values[2]
				{
					UINT32_MAX,
					UINT32_MAX
				};
			};

			[[nodiscard]] explicit operator bool() const;
		};

		struct Settings final
		{
			std::function<bool(const DeviceInfo& info)> deviceSuitableFunc;
			std::function<uint32_t(const DeviceInfo& info)> deviceRatingFunc;
		};

		struct Info final
		{
			const Settings& settings;
			const std::vector<const char*>& deviceExtensions;
			const VkInstance instance;
			const VkSurfaceKHR surface;
			VkPhysicalDevice& physicalDevice;
		};

		explicit PhysicalDeviceFactory(const Info& info);

		[[nodiscard]] static QueueFamilies GetQueueFamilies(const VkSurfaceKHR& surface, VkPhysicalDevice physicalDevice);

	private:
		[[nodiscard]] static bool IsDeviceSuitable(const Info& info, const DeviceInfo& deviceInfo);
		[[nodiscard]] static uint32_t RateDevice(const Info& info, const DeviceInfo& deviceInfo);
		[[nodiscard]] static bool CheckDeviceExtensionSupport(VkPhysicalDevice device,
			const std::vector<const char*>& extensions);
	};
}
