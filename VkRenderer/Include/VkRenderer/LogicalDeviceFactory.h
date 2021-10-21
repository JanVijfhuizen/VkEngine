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
			const VkPhysicalDevice physicalDevice;
			const VkSurfaceKHR surface;
			const Debugger& debugger;
			VkDevice& device;
			Queues& queues;
		};

		explicit LogicalDeviceFactory(const Info& info);
	};
}
