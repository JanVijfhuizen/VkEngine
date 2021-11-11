#pragma once
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/SwapChain.h"
#include "Mesh.h"
#include "Texture.h"

struct DepthBuffer;

namespace vi
{
	class WindowSystemGLFW;
}

class RenderSystem final
{
public:
	typedef Singleton<RenderSystem> Instance;

	RenderSystem();
	~RenderSystem();

	void BeginFrame(bool* quit);
	void EndFrame();

	template <typename Vert = Vertex2d, typename Ind = uint16_t>
	[[nodiscard]] Mesh CreateMesh(const std::vector<Vert>& vertices, const std::vector<Ind>& indices);
	void UseMesh(const Mesh& mesh) const;
	void DestroyMesh(const Mesh& mesh);

	[[nodiscard]] Texture CreateTexture(const std::string& fileName);
	void DestroyTexture(const Texture& texture);

	[[nodiscard]] DepthBuffer CreateDepthBuffer(glm::ivec2 resolution);
	void DestroyDepthBuffer(DepthBuffer& depthBuffer) const;

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

template <typename Vert, typename Ind>
Mesh RenderSystem::CreateMesh(const std::vector<Vert>& vertices, const std::vector<Ind>& indices)
{
	auto cpyCommandBuffer = _vkRenderer.CreateCommandBuffer();
	const auto cpyFence = _vkRenderer.CreateFence();

	// Send vertex data.
	const auto vertStagingBuffer = _vkRenderer.CreateBuffer<Vert>(vertices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto vertStagingMem = _vkRenderer.AllocateMemory(vertStagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	_vkRenderer.BindMemory(vertStagingBuffer, vertStagingMem);
	_vkRenderer.MapMemory(vertStagingMem, vertices.data(), 0, vertices.size());

	const auto vertBuffer = _vkRenderer.CreateBuffer<Vert>(vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	const auto vertMem = _vkRenderer.AllocateMemory(vertBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_vkRenderer.BindMemory(vertBuffer, vertMem);

	// Send indices data.
	const auto indStagingBuffer = _vkRenderer.CreateBuffer<Ind>(indices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto indStagingMem = _vkRenderer.AllocateMemory(indStagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	_vkRenderer.BindMemory(indStagingBuffer, indStagingMem);
	_vkRenderer.MapMemory(indStagingMem, indices.data(), 0, indices.size());

	const auto indBuffer = _vkRenderer.CreateBuffer<Vert>(vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	const auto indMem = _vkRenderer.AllocateMemory(indBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_vkRenderer.BindMemory(indBuffer, indMem);

	_vkRenderer.BeginCommandBufferRecording(cpyCommandBuffer);
	_vkRenderer.CopyBuffer(vertStagingBuffer, vertBuffer, vertices.size() * sizeof(Vert));
	_vkRenderer.CopyBuffer(indStagingBuffer, indBuffer, indices.size() * sizeof(Ind));
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
	mesh.indCount = indices.size();

	return mesh;
}
