#include "pch.h"
#include "Light3d.h"
#include "VkRenderer/RenderPassInfo.h"
#include "FileReader.h"
#include "VkRenderer/PipelineInfo.h"
#include "Transform3d.h"
#include "ShadowCaster.h"
#include "VkRenderer/WindowSystemGLFW.h"
#include "VkRenderer/DescriptorLayoutInfo.h"
#include "Camera3d.h"

Light3d::System::System(const uint32_t size, const Info& info) : ShaderSet<Light3d, Frame>(size), _info(info)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& swapChain = renderSystem.GetSwapChain();

	vi::RenderPassInfo renderPassInfo{};
	renderPassInfo.useDepthAttachment = true;
	_renderPass = renderer.CreateRenderPass(renderPassInfo);

	const auto vertCode = FileReader::Read("Shaders/vert3dShadow.spv");

	_vertModule = renderer.CreateShaderModule(vertCode);

	vi::DescriptorLayoutInfo lightLayoutInfo{};
	_bindingInfo.size = sizeof Ubo;
	_bindingInfo.flag = VK_SHADER_STAGE_VERTEX_BIT;
	lightLayoutInfo.bindings.push_back(_bindingInfo);
	auto layout = renderer.CreateLayout(lightLayoutInfo);

	vi::PipelineLayoutInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex3d::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex3d::GetBindingDescription();
	pipelineInfo.setLayouts.push_back(layout);
	pipelineInfo.modules.push_back(
		{
			_vertModule,
			VK_SHADER_STAGE_VERTEX_BIT
		});
	pipelineInfo.pushConstants.push_back(
		{
			sizeof Transform3d::Baked,
			VK_SHADER_STAGE_VERTEX_BIT
		});
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.extent = 
	{ 
		static_cast<uint32_t>(info.shadowResolution.x), 
		static_cast<uint32_t>(info.shadowResolution.y)
	};

	_pipeline = renderer.CreatePipeline(pipelineInfo);
	_commandBuffer = renderer.CreateCommandBuffer();
	_fence = renderer.CreateFence();

	const uint32_t imageCount = swapChain.GetImageCount();
	VkDescriptorType uboType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	_descriptorPool.Construct(imageCount * GetSize(), layout, &uboType, 1);
}

void Light3d::System::Cleanup()
{
	ShaderSet<Light3d, Frame>::Cleanup();

	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	renderer.DestroyPipeline(_pipeline);
	renderer.DestroyShaderModule(_vertModule);
	renderer.DestroyRenderPass(_renderPass);
	renderer.DestroyCommandBuffer(_commandBuffer);
	renderer.DestroyFence(_fence);
	_descriptorPool.Cleanup();
}

void Light3d::System::Update()
{
	ShaderSet<Light3d, Frame>::Update();

	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	const auto frames = GetCurrentFrameSet().Get<Frame>();
	auto& shadowCasters = ShadowCaster::System::Instance::Get();
	auto& meshes = Mesh::System::Instance::Get();
	auto& transforms = Transform3d::System::Instance::Get();
	const auto bakedTransforms = transforms.GetSets()[0].Get<Transform3d::Baked>();
	auto& camera = Camera3d::System::Instance::Get()[0];

	VkClearValue clearValue{};
	clearValue.depthStencil = { 1, 0 };

	renderer.BeginCommandBufferRecording(_commandBuffer);
	renderer.BindPipeline(_pipeline);

	for (const auto [light, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);
		auto& frame = frames[denseId];
		auto& transform = transforms[sparseId];

		renderer.BeginRenderPass(frame.frameBuffer, _renderPass, {}, _info.shadowResolution, &clearValue, 1);

		glm::mat4 view = glm::lookAt(transform.position, camera.lookat, glm::vec3(0, 1, 0));
		glm::mat4 projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, .1f, 1000.f);

		Ubo ubo{};
		ubo.lightSpaceMatrix = projection * view;
		renderer.MapMemory(frame.lightMemory, &ubo, 0, 1);

		for (const auto [shadowCaster, shadowCasterSparseId] : shadowCasters)
		{
			auto& mesh = meshes[shadowCasterSparseId];
			auto& bakedTransform = bakedTransforms[transforms.GetDenseId(shadowCasterSparseId)];

			renderSystem.UseMesh(mesh);
			renderer.BindDescriptorSets(&frame.descriptorSet, 1);
			renderer.UpdatePushConstant(_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, bakedTransform);
			renderer.Draw(mesh.indCount);
		}

		renderer.EndRenderPass();
	}

	renderer.EndCommandBufferRecording();
	renderer.Submit(&_commandBuffer, 1, nullptr, nullptr, _fence);
	renderer.WaitForFence(_fence);
}

void Light3d::System::ConstructInstanceFrame(Frame& frame, Light3d&, const uint32_t)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	frame.depthBuffer = renderSystem.CreateDepthBuffer(_info.shadowResolution);
	frame.frameBuffer = renderer.CreateFrameBuffer(&frame.depthBuffer.imageView, 1, _renderPass, 
		{ 
			static_cast<uint32_t>(_info.shadowResolution.x), 
			static_cast<uint32_t>(_info.shadowResolution.y)
		});

	frame.descriptorSet = _descriptorPool.Get();
	frame.lightBuffer = renderer.CreateBuffer<Ubo>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	frame.lightMemory = renderer.AllocateMemory(frame.lightBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(frame.lightBuffer, frame.lightMemory);
	renderer.BindBuffer(frame.descriptorSet, frame.lightBuffer, _bindingInfo, 0, 0);
}

void Light3d::System::CleanupInstanceFrame(Frame& frame, Light3d&, const uint32_t)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	_descriptorPool.Add(frame.descriptorSet);
	renderSystem.DestroyDepthBuffer(frame.depthBuffer);
	renderer.DestroyFrameBuffer(frame.frameBuffer);
	renderer.DestroyBuffer(frame.lightBuffer);
	renderer.FreeMemory(frame.lightMemory);
}
