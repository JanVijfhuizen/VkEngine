#pragma once

namespace vi
{
	class PhysicalDeviceFactory final
	{
	public:
		struct Info;

		struct DeviceInfo final
		{
			VkPhysicalDeviceProperties& properties;
			VkPhysicalDeviceFeatures& features;
		};

		struct Settings final
		{
			std::function<bool(const DeviceInfo& info)> deviceSuitableFunc;
			std::function<uint32_t(const DeviceInfo& info)> deviceRatingFunc;
		};

		struct Info final
		{
			Settings settings;
			VkInstance instance;
			VkSurfaceKHR surface;
			VkPhysicalDevice& physicalDevice;
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

				uint32_t families[2]
				{
					UINT32_MAX, 
					UINT32_MAX
				};
			};

			[[nodiscard]] explicit operator bool() const;
		};

		explicit PhysicalDeviceFactory(const Info& info);

	private:
		[[nodiscard]] static bool IsDeviceSuitable(const Info& info, const DeviceInfo& deviceInfo);
		[[nodiscard]] static uint32_t RateDevice(const Info& info, const DeviceInfo& deviceInfo);
		[[nodiscard]] static QueueFamilies GetQueueFamilies(const Info& info, VkPhysicalDevice physicalDevice);
	};
}
