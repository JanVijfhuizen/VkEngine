#include "pch.h"
#include "RenderSystem.h"
#include "VkRenderer/WindowSystemGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/RenderPassInfo.h"
#include "TextureLoader.h"
#include "DepthBuffer.h"

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

void RenderSystem::UseMesh(const Mesh& mesh) const
{
	_vkRenderer.BindVertexBuffer(mesh.vertexBuffer);
	_vkRenderer.BindIndicesBuffer(mesh.indexBuffer);
}

void RenderSystem::DestroyMesh(const Mesh& mesh)
{
	_vkRenderer.DestroyBuffer(mesh.vertexBuffer);
	_vkRenderer.FreeMemory(mesh.vertexMemory);
	_vkRenderer.DestroyBuffer(mesh.indexBuffer);
	_vkRenderer.FreeMemory(mesh.indexMemory);
}

Texture RenderSystem::CreateTexture(const std::string& fileName)
{
	int32_t w, h, d;
	const auto tex = TextureLoader::Load("Textures/" + fileName, w, h, d);
	const auto texStagingBuffer = _vkRenderer.CreateBuffer<unsigned char>(w * h * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto texStagingMem = _vkRenderer.AllocateMemory(texStagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	_vkRenderer.BindMemory(texStagingBuffer, texStagingMem);
	_vkRenderer.MapMemory(texStagingMem, tex, 0, w * h * 4);
	TextureLoader::Free(tex);

	const auto img = _vkRenderer.CreateImage({ w, h });
	const auto imgMem = _vkRenderer.AllocateMemory(img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_vkRenderer.BindMemory(img, imgMem);

	auto imgCmd = _vkRenderer.CreateCommandBuffer();
	_vkRenderer.BeginCommandBufferRecording(imgCmd);
	_vkRenderer.TransitionImageLayout(img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	_vkRenderer.EndCommandBufferRecording();

	const auto imgFence = _vkRenderer.CreateFence();
	_vkRenderer.Submit(&imgCmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, imgFence);
	_vkRenderer.WaitForFence(imgFence);

	_vkRenderer.BeginCommandBufferRecording(imgCmd);
	_vkRenderer.CopyBuffer(texStagingBuffer, img, w, h);
	_vkRenderer.EndCommandBufferRecording();
	_vkRenderer.Submit(&imgCmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, imgFence);
	_vkRenderer.WaitForFence(imgFence);

	_vkRenderer.BeginCommandBufferRecording(imgCmd);
	_vkRenderer.TransitionImageLayout(img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	_vkRenderer.EndCommandBufferRecording();
	_vkRenderer.Submit(&imgCmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, imgFence);
	_vkRenderer.WaitForFence(imgFence);

	_vkRenderer.DestroyFence(imgFence);
	_vkRenderer.DestroyCommandBuffer(imgCmd);
	_vkRenderer.DestroyBuffer(texStagingBuffer);
	_vkRenderer.FreeMemory(texStagingMem);

	const auto imgView = _vkRenderer.CreateImageView(img);

	Texture texture{};
	texture.resolution = { w, h };
	texture.channels = d;
	texture.image = img;
	texture.imageMemory = imgMem;
	texture.imageView = imgView;

	return texture;
}

void RenderSystem::DestroyTexture(const Texture& texture)
{
	_vkRenderer.DestroyImageView(texture.imageView);
	_vkRenderer.DestroyImage(texture.image);
	_vkRenderer.FreeMemory(texture.imageMemory);
}

DepthBuffer RenderSystem::CreateDepthBuffer(const glm::ivec2 resolution) const
{
	const auto format = _vkRenderer.FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	DepthBuffer depthBuffer{};
	depthBuffer.image = _vkRenderer.CreateImage(resolution, format, VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depthBuffer.imageMemory = _vkRenderer.AllocateMemory(depthBuffer.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_vkRenderer.BindMemory(depthBuffer.image, depthBuffer.imageMemory);
	depthBuffer.imageView = _vkRenderer.CreateImageView(depthBuffer.image, format, VK_IMAGE_ASPECT_DEPTH_BIT);

	return depthBuffer;
}

void RenderSystem::DestroyDepthBuffer(DepthBuffer& depthBuffer) const
{
	_vkRenderer.DestroyImageView(depthBuffer.imageView);
	_vkRenderer.DestroyImage(depthBuffer.image);
	_vkRenderer.FreeMemory(depthBuffer.imageMemory);
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
