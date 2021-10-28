#pragma once
#include "VkRenderer/VkRenderer.h"
#include "VkRenderer/SwapChain.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

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

	[[nodiscard]] Mesh CreateMesh(const Mesh::Info& info);
	void UseMesh(const Mesh& mesh) const;
	void DestroyMesh(const Mesh& mesh);

	[[nodiscard]] Texture CreateTexture(const std::string& fileName);
	void DestroyTexture(const Texture& texture);

	[[nodiscard]] Material CreateMaterial() const;
	void DestroyMaterial(const Material& material) const;

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
