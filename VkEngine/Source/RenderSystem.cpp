#include "pch.h"
#include "RenderSystem.h"
#include "VkRenderer/WindowSystemGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/RenderPassInfo.h"

RenderSystem::RenderSystem()
{
	vi::WindowSystemGLFW::VkInfo windowInfo{};
	windowInfo.name = "Prototype Game";
	windowInfo.resolution = {800, 600};
	_windowSystem = new vi::WindowSystemGLFW(windowInfo);

	vi::VkRenderer::Settings settings;
	settings.debugger.validationLayers.push_back("VK_LAYER_RENDERDOC_Capture");
	_vkRenderer.Construct(*_windowSystem, settings);

	_swapChain.Construct(_vkRenderer);

	vi::RenderPassInfo renderPassInfo{};
	const vi::RenderPassInfo::Attachment renderPassAttachment{};
	renderPassInfo.attachments.push_back(renderPassAttachment);
	renderPassInfo.format = _swapChain.GetFormat();
	_renderPass = _vkRenderer.CreateRenderPass(renderPassInfo);

	_swapChain.SetRenderPass(_renderPass);
}

RenderSystem::~RenderSystem()
{
	_swapChain.Cleanup();
	_vkRenderer.DestroyRenderPass(_renderPass);
	_vkRenderer.Cleanup();
	delete _windowSystem;
}

void RenderSystem::BeginFrame(bool* quit)
{
	_windowSystem->BeginFrame(*quit);
	if (*quit)
		return;

	_swapChain.GetNext(_image, _frame);

	const auto extent = _swapChain.GetExtent();
	_vkRenderer.BeginCommandBufferRecording(_image.commandBuffer);
	_vkRenderer.BeginRenderPass(_image.frameBuffer, _swapChain.GetRenderPass(), {}, { extent.width, extent.height });
}

void RenderSystem::EndFrame()
{
	_vkRenderer.EndRenderPass();
	_vkRenderer.EndCommandBufferRecording();
	_vkRenderer.Submit(&_image.commandBuffer, 1, _frame.imageAvailableSemaphore, _frame.renderFinishedSemaphore, _frame.inFlightFence);
	const auto result = _swapChain.Present();
	if (!result)
	{
		// Recreate assets.
	}
}

vi::WindowSystemGLFW& RenderSystem::GetWindowSystem() const
{
	return *_windowSystem;
}

vi::VkRenderer& RenderSystem::GetVkRenderer()
{
	return _vkRenderer;
}

vi::SwapChain& RenderSystem::GetSwapChain()
{
	return _swapChain;
}
