#pragma once

namespace vi
{
	class VkRenderer;

	class InstanceFactory final
	{
	public:
		explicit InstanceFactory(VkRenderer& renderer);
		static void Cleanup(VkRenderer& renderer);

	private:
		[[nodiscard]] static VkApplicationInfo CreateApplicationInfo(VkRenderer& renderer);
		[[nodiscard]] static std::vector<const char*> GetExtensions(VkRenderer& renderer);
	};
}