#include "pch.h"
#include "VkRenderer.h"
#include "WindowSystem.h"
#include "InstanceFactory.h"

namespace vi
{
	VkRenderer::VkRenderer(WindowSystem& system) : _windowSystem(system)
	{
		const InstanceFactory::Info instanceInfo
		{
			_windowSystem,
			_debugger,
			_instance
		};

		InstanceFactory{ instanceInfo };
		_debugger.Construct(_instance);
		_windowSystem.CreateSurface(_instance, _surface);
	}

	VkRenderer::~VkRenderer()
	{
		_debugger.Cleanup();

		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyInstance(_instance, nullptr);
	}
}
