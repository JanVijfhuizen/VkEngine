#include "pch.h"
#include "VkRenderer.h"
#include "WindowSystem.h"
#include "InstanceFactory.h"
#include "LogicalDeviceFactory.h"

namespace vi
{
	VkRenderer::VkRenderer(WindowSystem& system, const Settings& settings) : _windowSystem(system)
	{
		const InstanceFactory::Info instanceInfo
		{
			_windowSystem,
			_debugger,
			_instance
		};
		InstanceFactory{instanceInfo};

		_debugger.Construct(settings.debugger, _instance);
		_windowSystem.CreateSurface(_instance, _surface);

		const PhysicalDeviceFactory::Info physicalDeviceInfo
		{
			settings.physicalDevice,
			settings.deviceExtensions,
			_instance,
			_surface,
			_physicalDevice,
		};
		PhysicalDeviceFactory{physicalDeviceInfo};

		const LogicalDeviceFactory::Info logicalDeviceInfo
		{
			settings.deviceExtensions,
			_physicalDevice,
			_surface,
			_debugger,
			_device,
			_queues
		};
		LogicalDeviceFactory{logicalDeviceInfo};
	}

	VkRenderer::~VkRenderer()
	{
		vkDestroyDevice(_device, nullptr);
		_debugger.Cleanup();

		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyInstance(_instance, nullptr);
	}
}
