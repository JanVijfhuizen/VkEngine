#pragma once

namespace vi
{
	class VkRenderer;

	class LogicalDeviceFactory final
	{
	public:
		explicit LogicalDeviceFactory(VkRenderer& renderer);
		static void Cleanup(VkRenderer& renderer);
	};
}
