#pragma once

namespace vi
{
	class WindowSystem
	{
	public:
		struct VkInfo final
		{
			glm::ivec2 resolution{ 800, 600 };
			std::string name{ "My Window" };
		};

		virtual void CreateSurface(VkInstance instance, VkSurfaceKHR& surface) = 0;
		[[nodiscard]] virtual const VkInfo& GetVkInfo() const = 0;
		[[nodiscard]] virtual bool QueryHasResized() = 0;
		virtual void GetRequiredExtensions(std::vector<const char*>& extensions) = 0;
	};
}