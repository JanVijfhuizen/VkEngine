﻿#include "pch.h"
#include "RenderSystem.h"
#include "VkRenderer/WindowSystemGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/RenderPassInfo.h"
#include "TextureLoader.h"

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

Mesh RenderSystem::CreateMesh(const Mesh::Info& info)
{
	auto cpyCommandBuffer = _vkRenderer.CreateCommandBuffer();
	const auto cpyFence = _vkRenderer.CreateFence();

	// Send vertex data.
	const auto vertStagingBuffer = _vkRenderer.CreateBuffer<Vertex>(info.vertices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto vertStagingMem = _vkRenderer.AllocateMemory(vertStagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	_vkRenderer.BindMemory(vertStagingBuffer, vertStagingMem);
	_vkRenderer.MapMemory(vertStagingMem, info.vertices.data(), 0, info.vertices.size());

	const auto vertBuffer = _vkRenderer.CreateBuffer<Vertex>(info.vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	const auto vertMem = _vkRenderer.AllocateMemory(vertBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_vkRenderer.BindMemory(vertBuffer, vertMem);

	// Send indices data.
	const auto indStagingBuffer = _vkRenderer.CreateBuffer<uint16_t>(info.indices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto indStagingMem = _vkRenderer.AllocateMemory(indStagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	_vkRenderer.BindMemory(indStagingBuffer, indStagingMem);
	_vkRenderer.MapMemory(indStagingMem, info.indices.data(), 0, info.indices.size());

	const auto indBuffer = _vkRenderer.CreateBuffer<Vertex>(info.vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	const auto indMem = _vkRenderer.AllocateMemory(indBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_vkRenderer.BindMemory(indBuffer, indMem);

	_vkRenderer.BeginCommandBufferRecording(cpyCommandBuffer);
	_vkRenderer.CopyBuffer(vertStagingBuffer, vertBuffer, info.vertices.size() * sizeof(Vertex));
	_vkRenderer.CopyBuffer(indStagingBuffer, indBuffer, info.indices.size() * sizeof(int16_t));
	_vkRenderer.EndCommandBufferRecording();

	_vkRenderer.Submit(&cpyCommandBuffer, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, cpyFence);
	_vkRenderer.WaitForFence(cpyFence);

	_vkRenderer.DestroyBuffer(vertStagingBuffer);
	_vkRenderer.FreeMemory(vertStagingMem);

	_vkRenderer.DestroyBuffer(indStagingBuffer);
	_vkRenderer.FreeMemory(indStagingMem);

	_vkRenderer.DestroyFence(cpyFence);
	_vkRenderer.DestroyCommandBuffer(cpyCommandBuffer);

	Mesh mesh{};
	mesh.vertexBuffer = vertBuffer;
	mesh.vertexMemory = vertMem;
	mesh.indexBuffer = indBuffer;
	mesh.indexMemory = indMem;
	mesh.indCount = info.indices.size();

	return mesh;
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

Material RenderSystem::CreateMaterial() const
{
	return {};
}

void RenderSystem::DestroyMaterial(const Material& material) const
{
	
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
