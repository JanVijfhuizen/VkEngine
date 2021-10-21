#pragma once
#include "WindowSystem.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vi
{
	class WindowSystemGLFW final : public WindowSystem
	{
	public:
		explicit WindowSystemGLFW(VkInfo info = {});
		~WindowSystemGLFW();

		void BeginFrame(bool& outQuit) const;

		void CreateSurface(VkInstance instance, VkSurfaceKHR& surface) override;
		[[nodiscard]] const VkInfo& GetVkInfo() const override;
		bool QueryHasResized() override;
		void GetRequiredExtensions(std::vector<const char*>& extensions) override;

	private:
		VkInfo _info;
		GLFWwindow* _window;
		bool _resized = false;

		static void FramebufferResizeCallback(struct GLFWwindow* window, int width, int height);
	};
}