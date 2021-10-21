#pragma once
#include "Debugger.h"
#include "Queues.h"

namespace vi
{
	class LogicalDeviceFactory final
	{
	public:
		struct Info final
		{
			const std::vector<const char*>& deviceExtensions;
			const VkPhysicalDevice physicalDevice;
			const VkSurfaceKHR surface;
			const Debugger& debugger;
			VkDevice& device;
			Queues& queues;
		};

		explicit LogicalDeviceFactory(const Info& info);
	};
}
