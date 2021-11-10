#pragma once
#include "ShaderSet.h"
#include "DescriptorPool.h"
#include "VkRenderer/BindingInfo.h"
#include "VkRenderer/DescriptorLayoutInfo.h"

struct CameraFrame final
{
	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDescriptorSet descriptor;
};

template <typename Camera, typename Ubo>
class CameraSystem : public ShaderSet<Camera, CameraFrame>
{
public:
	typedef Singleton<CameraSystem> Instance;

	explicit CameraSystem(uint32_t size);
	void Cleanup() override;
	void Update() override;

	[[nodiscard]] VkDescriptorSetLayout GetLayout() const;

protected:
	[[nodiscard]] virtual Ubo CreateUbo(Camera& camera, uint32_t index) = 0;

private:
	void ConstructInstanceFrame(CameraFrame& frame, Camera& material, uint32_t denseId) override;
	void CleanupInstanceFrame(CameraFrame& frame, Camera& material, uint32_t denseId) override;

	vi::BindingInfo _bindingInfo{};
	VkDescriptorSetLayout _descriptorLayout;
	DescriptorPool _descriptorPool;
};

template <typename Camera, typename Ubo>
CameraSystem<Camera, Ubo>::CameraSystem(const uint32_t size) : ShaderSet<Camera, CameraFrame>(size)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& swapChain = renderSystem.GetSwapChain();

	vi::DescriptorLayoutInfo camLayoutInfo{};
	_bindingInfo.size = sizeof Ubo;
	_bindingInfo.flag = VK_SHADER_STAGE_VERTEX_BIT;
	camLayoutInfo.bindings.push_back(_bindingInfo);
	_descriptorLayout = renderer.CreateLayout(camLayoutInfo);

	const uint32_t imageCount = swapChain.GetImageCount();
	VkDescriptorType uboType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	_descriptorPool.Construct(imageCount * ShaderSet<Camera, CameraFrame>::GetSize(), _descriptorLayout, &uboType, 1);
}

template <typename Camera, typename Ubo>
void ::CameraSystem<Camera, Ubo>::Cleanup()
{
	ShaderSet<Camera, CameraFrame>::Cleanup();

	_descriptorPool.Cleanup();
}

template <typename Camera, typename Ubo>
void ::CameraSystem<Camera, Ubo>::Update()
{
	ShaderSet<Camera, CameraFrame>::Update();

	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto frames = ShaderSet<Camera, CameraFrame>::GetCurrentFrameSet();

	for (const auto [instance, sparseId] : *this)
	{
		const uint32_t denseId = ShaderSet<Camera, CameraFrame>::GetDenseId(sparseId);
		auto& frame = frames.template Get<CameraFrame>(denseId);

		Ubo ubo = CreateUbo(instance, sparseId);
		renderer.MapMemory(frame.memory, &ubo, 0, 1);
	}
}

template <typename Camera, typename Ubo>
VkDescriptorSetLayout CameraSystem<Camera, Ubo>::GetLayout() const
{
	return _descriptorLayout;
}

template <typename Camera, typename Ubo>
void ::CameraSystem<Camera, Ubo>::ConstructInstanceFrame(CameraFrame& frame, Camera&, const uint32_t)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	frame.descriptor = _descriptorPool.Get();

	frame.buffer = renderer.CreateBuffer<Ubo>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	frame.memory = renderer.AllocateMemory(frame.buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(frame.buffer, frame.memory);
	renderer.BindBuffer(frame.descriptor, frame.buffer, _bindingInfo, 0, 0);
}

template <typename Camera, typename Ubo>
void ::CameraSystem<Camera, Ubo>::CleanupInstanceFrame(CameraFrame& frame, Camera&, const uint32_t)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	_descriptorPool.Add(frame.descriptor);
	renderer.FreeMemory(frame.memory);
	renderer.DestroyBuffer(frame.buffer);
}
