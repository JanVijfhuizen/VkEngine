#include "pch.h"
#include "WindowSystemGLFW.h"
#include <utility>

namespace vi
{
	WindowSystemGLFW::WindowSystemGLFW(VkInfo info) : _info(std::move(info))
	{
		const auto& resolution = info.resolution;

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		_window = glfwCreateWindow(resolution.x, resolution.y, info.name.c_str(), nullptr, nullptr);
		assert(_window);
		glfwSetWindowUserPointer(_window, this);
		glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);
	}

	WindowSystemGLFW::~WindowSystemGLFW()
	{
		glfwDestroyWindow(_window);
		glfwTerminate();
	}

	void WindowSystemGLFW::BeginFrame(bool& outQuit) const
	{
		outQuit = glfwWindowShouldClose(_window);
		if (outQuit)
			return;

		glfwPollEvents();

		int32_t width = 0, height = 0;
		glfwGetFramebufferSize(_window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(_window, &width, &height);
			glfwWaitEvents();
		}
	}

	void WindowSystemGLFW::CreateSurface(const VkInstance instance, VkSurfaceKHR& surface)
	{
		const auto result = glfwCreateWindowSurface(instance, _window, nullptr, &surface);
		assert(!result);
	}

	const WindowSystem::VkInfo& WindowSystemGLFW::GetVkInfo() const
	{
		return _info;
	}

	bool WindowSystemGLFW::QueryHasResized()
	{
		const bool resized = _resized;
		_resized = false;
		return resized;
	}

	void WindowSystemGLFW::GetRequiredExtensions(std::vector<const char*>& extensions)
	{
		uint32_t glfwExtensionCount = 0;
		const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (uint32_t i = 0; i < glfwExtensionCount; ++i)
			extensions.push_back(glfwExtensions[i]);
	}

	void WindowSystemGLFW::FramebufferResizeCallback(struct GLFWwindow* window, int width, int height)
	{
		auto self = reinterpret_cast<WindowSystemGLFW*>(glfwGetWindowUserPointer(window));
		self->_resized = true;
		self->_info.resolution = { width, height };
	}
}