#pragma once
#include "ShaderSet.h"
#include "DescriptorPool.h"
#include "VkRenderer/BindingInfo.h"
#include "VkRenderer/DescriptorLayoutInfo.h"

struct alignas(4) Camera final
{
	struct Frame final
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorSet descriptor;
	};

	template <typename Ubo>
	class System : public ShaderSet<Camera, Frame>
	{
	public:
		typedef Singleton<System> Instance;

		explicit System(uint32_t size);
		void Cleanup() override;
		void Update() override;

		[[nodiscard]] VkDescriptorSetLayout GetLayout() const;
		[[nodiscard]] virtual Ubo CreateUbo(Camera& camera, uint32_t index) = 0;

	private:
		void ConstructInstanceFrame(Frame& frame, Camera& material, uint32_t denseId) override;
		void CleanupInstanceFrame(Frame& frame, Camera& material, uint32_t denseId) override;

		vi::BindingInfo _bindingInfo{};
		VkDescriptorSetLayout _descriptorLayout;
		DescriptorPool _descriptorPool;
	};
};

template <typename Ubo>
Camera::System<Ubo>::System(const uint32_t size) : ShaderSet<Camera, Frame>(size)
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
	_descriptorPool.Construct(imageCount * GetSize(), _descriptorLayout, &uboType, 1);
}

template <typename Ubo>
void Camera::System<Ubo>::Cleanup()
{
	ShaderSet<Camera, Frame>::Cleanup();

	_descriptorPool.Cleanup();
}

template <typename Ubo>
void Camera::System<Ubo>::Update()
{
	ShaderSet<Camera, Frame>::Update();

	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto frames = GetCurrentFrameSet();

	for (const auto [instance, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);
		auto& frame = frames.Get<Frame>(denseId);

		Ubo ubo = CreateUbo(instance, sparseId);
		renderer.MapMemory(frame.memory, &ubo, 0, 1);
	}
}

template <typename Ubo>
VkDescriptorSetLayout Camera::System<Ubo>::GetLayout() const
{
	return _descriptorLayout;
}

template <typename Ubo>
void Camera::System<Ubo>::ConstructInstanceFrame(Frame& frame, Camera&, const uint32_t)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	frame.descriptor = _descriptorPool.Get();

	frame.buffer = renderer.CreateBuffer<Ubo>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	frame.memory = renderer.AllocateMemory(frame.buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(frame.buffer, frame.memory);
	renderer.BindBuffer(frame.descriptor, frame.buffer, _bindingInfo, 0, 0);
}

template <typename Ubo>
void Camera::System<Ubo>::CleanupInstanceFrame(Frame& frame, Camera&, const uint32_t)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	_descriptorPool.Add(frame.descriptor);
	renderer.FreeMemory(frame.memory);
	renderer.DestroyBuffer(frame.buffer);
}
