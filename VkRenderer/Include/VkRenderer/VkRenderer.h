#pragma once
#include "Debugger.h"
#include "PhysicalDeviceFactory.h"

namespace vi
{
	class WindowSystem;

	class VkRenderer final
	{
	public:
		struct Settings final
		{
			PhysicalDeviceFactory::Settings physicalDevice{};
			Debugger::Settings debugger{};
		};

		explicit VkRenderer(class WindowSystem& system, const Settings& settings = {});
		~VkRenderer();

	private:
		WindowSystem& _windowSystem;
		Debugger _debugger{};

		VkInstance _instance;
		VkSurfaceKHR _surface;
		VkPhysicalDevice _physicalDevice;
		VkDevice _device;
	};
}
