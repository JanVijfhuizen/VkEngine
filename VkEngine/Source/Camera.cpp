#include "pch.h"
#include "Camera.h"
#include "VkRenderer/DescriptorLayoutInfo.h"
#include "Transform.h"
#include "VkRenderer/WindowSystemGLFW.h"

Camera::System::System(const uint32_t size) : ShaderSet<Camera, Frame>(size)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& swapChain = renderSystem.GetSwapChain();

	vi::DescriptorLayoutInfo camLayoutInfo{};
	_bindingInfo.size = sizeof Camera;
	_bindingInfo.flag = VK_SHADER_STAGE_VERTEX_BIT;
	camLayoutInfo.bindings.push_back(_bindingInfo);
	_descriptorLayout = renderer.CreateLayout(camLayoutInfo);

	const uint32_t imageCount = swapChain.GetImageCount();
	VkDescriptorType uboType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	_descriptorPool = renderer.CreateDescriptorPool(&uboType, 1, imageCount);
}

void Camera::System::Update()
{
	ShaderSet<Camera, Frame>::Update();

	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& windowSystem = renderSystem.GetWindowSystem();
	auto& transforms = Singleton<SparseSet<Transform>>::Get();
	auto frames = GetCurrentFrameSet();

	const auto resolution = windowSystem.GetVkInfo().resolution;
	const float aspectRatio = static_cast<float>(resolution.x) / resolution.y;

	for (const auto [instance, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);
		auto& frame = frames.Get<Frame>(denseId);
		auto& transform = transforms[sparseId];

		instance.aspectRatio = aspectRatio;
		instance.position = glm::vec3(transform.position, 1);

		renderer.MapMemory(frame.memory, &instance, 0, 1);
	}
}

void Camera::System::ConstructInstanceFrame(Frame& frame, Camera& material, const uint32_t denseId)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	renderer.CreateDescriptorSets(_descriptorPool, _descriptorLayout, &frame.descriptor, 1);

	frame.buffer = renderer.CreateBuffer<Camera>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	frame.memory = renderer.AllocateMemory(frame.buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(frame.buffer, frame.memory);
	renderer.BindBuffer(frame.descriptor, frame.buffer, _bindingInfo, 0, 0);
}

void Camera::System::CleanupInstanceFrame(Frame& frame, Camera& material, const uint32_t denseId)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	renderer.FreeMemory(frame.memory);
	renderer.DestroyBuffer(frame.buffer);
}