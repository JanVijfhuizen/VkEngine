#pragma once

namespace vi
{
	class VkRenderer;

	class CommandPoolFactory final
	{
	public:
		explicit CommandPoolFactory(VkRenderer& renderer);
		static void Cleanup(VkRenderer& renderer);
	};
}