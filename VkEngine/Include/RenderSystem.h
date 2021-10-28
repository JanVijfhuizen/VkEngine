#pragma once
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/SwapChain.h"

namespace vi
{
	class WindowSystemGLFW;
}

class RenderSystem final
{
public:
	RenderSystem();
	~RenderSystem();

	void BeginFrame(bool* quit);
	void EndFrame();

	[[nodiscard]] vi::WindowSystemGLFW& GetWindowSystem() const;
	[[nodiscard]] vi::VkRenderer& GetVkRenderer();
	[[nodiscard]] vi::SwapChain& GetSwapChain();

private:
	vi::WindowSystemGLFW* _windowSystem;
	vi::VkRenderer _vkRenderer{};
	vi::SwapChain _swapChain{};

	VkRenderPass _renderPass;

	vi::SwapChain::Image _image;
	vi::SwapChain::Frame _frame;
};
