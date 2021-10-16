#pragma once
#include "Debugger.h"

namespace vi
{
	class WindowSystem;

	class VkRenderer final
	{
	public:
		explicit VkRenderer(class WindowSystem& system);
		~VkRenderer();

	private:
		WindowSystem& _windowSystem;
		Debugger _debugger{};

		VkInstance _instance;
		VkSurfaceKHR _surface;
	};
}
