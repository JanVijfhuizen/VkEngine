#include "pch.h"
#include "VkRenderer.h"
#include "WindowSystem.h"
#include "InstanceFactory.h"
#include "LogicalDeviceFactory.h"

namespace vi
{
	VkRenderer::VkRenderer(WindowSystem& system, const Settings& settings) :  _windowSystem(system)
	{
		_settings = std::make_unique<Settings>();
		*_settings = settings;

		const InstanceFactory::Info instanceInfo
		{
			_windowSystem,
			_debugger,
			_instance
		};
		InstanceFactory{instanceInfo};

		_debugger.Construct(_settings->debugger, _instance);
		_windowSystem.CreateSurface(_instance, _surface);

		const PhysicalDeviceFactory::Info physicalDeviceInfo
		{
			_settings->physicalDevice,
			_settings->deviceExtensions,
			_instance,
			_surface,
			_physicalDevice,
		};
		PhysicalDeviceFactory{physicalDeviceInfo};

		const LogicalDeviceFactory::Info logicalDeviceInfo
		{
			_settings->deviceExtensions,
			_physicalDevice,
			_surface,
			_debugger,
			_device,
			_queues
		};
		LogicalDeviceFactory{logicalDeviceInfo};

		CreateSwapChainDependencies();
	}

	VkRenderer::~VkRenderer()
	{
		CleanupSwapChainDependendies();

		vkDestroyDevice(_device, nullptr);
		_debugger.Cleanup();

		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyInstance(_instance, nullptr);
	}

	void VkRenderer::CreateSwapChainDependencies()
	{
		const SwapChain::Info swapChainInfo
		{
			_physicalDevice,
			_surface,
			_device,
			&_windowSystem
		};

		_swapChain.Construct(swapChainInfo);
	}

	void VkRenderer::CleanupSwapChainDependendies() const
	{
		_swapChain.Cleanup();
	}
}
