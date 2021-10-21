#pragma once
#include "Debugger.h"
#include "PhysicalDeviceFactory.h"
#include "Queues.h"
#include "SwapChain.h"

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

			std::vector<const char*> deviceExtensions =
			{
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
		};

		explicit VkRenderer(class WindowSystem& system, const Settings& settings = {});
		~VkRenderer();

	private:
		std::unique_ptr<Settings> _settings{};

		WindowSystem& _windowSystem;
		Debugger _debugger{};
		SwapChain _swapChain{};

		VkInstance _instance;
		VkSurfaceKHR _surface;
		VkPhysicalDevice _physicalDevice;
		VkDevice _device;
		Queues _queues;

		void CreateSwapChainDependencies();
		void CleanupSwapChainDependendies();
	};
}
