#include "pch.h"
#include "Camera2d.h"
#include "VkRenderer/DescriptorLayoutInfo.h"
#include "Transform2d.h"
#include "VkRenderer/WindowSystemGLFW.h"

Camera2d::System::System(const uint32_t size) : ShaderSet<Camera2d, Frame>(size)
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

void Camera2d::System::Cleanup()
{
	ShaderSet<Camera2d, Frame>::Cleanup();

	_descriptorPool.Cleanup();
}

void Camera2d::System::Update()
{
	ShaderSet<Camera2d, Frame>::Update();

	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& windowSystem = renderSystem.GetWindowSystem();
	auto& transforms = Transform2d::System::Instance::Get();
	auto frames = GetCurrentFrameSet();

	const auto resolution = windowSystem.GetVkInfo().resolution;
	const float aspectRatio = static_cast<float>(resolution.x) / resolution.y;

	for (const auto [instance, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);
		auto& frame = frames.Get<Frame>(denseId);
		auto& transform = transforms[sparseId];

		Ubo ubo{};
		ubo.aspectRatio = aspectRatio;
		ubo.position = glm::vec3(transform.position, 1);

		renderer.MapMemory(frame.memory, &ubo, 0, 1);
	}
}

VkDescriptorSetLayout Camera2d::System::GetLayout() const
{
	return _descriptorLayout;
}

void Camera2d::System::ConstructInstanceFrame(Frame& frame, Camera2d&, const uint32_t)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	frame.descriptor = _descriptorPool.Get();

	frame.buffer = renderer.CreateBuffer<Ubo>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	frame.memory = renderer.AllocateMemory(frame.buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(frame.buffer, frame.memory);
	renderer.BindBuffer(frame.descriptor, frame.buffer, _bindingInfo, 0, 0);
}

void Camera2d::System::CleanupInstanceFrame(Frame& frame, Camera2d&, const uint32_t)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	_descriptorPool.Add(frame.descriptor);
	renderer.FreeMemory(frame.memory);
	renderer.DestroyBuffer(frame.buffer);
}