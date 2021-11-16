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
	renderPassInfo.depthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	_renderPass = renderer.CreateRenderPass(renderPassInfo);

	const auto vertCode = FileReader::Read("Shaders/vert3dShadow.spv");

	_vertModule = renderer.CreateShaderModule(vertCode);

	vi::DescriptorLayoutInfo lsmLayoutInfo{};
	_lsmBindingInfo.size = sizeof Ubo;
	_lsmBindingInfo.flag = VK_SHADER_STAGE_VERTEX_BIT;
	lsmLayoutInfo.bindings.push_back(_lsmBindingInfo);
	auto layout = renderer.CreateLayout(lsmLayoutInfo);

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
	_lsmDescriptorPool.Construct(imageCount * GetSize(), layout, &uboType, 1);

	vi::DescriptorLayoutInfo depthLayoutInfo{};
	depthLayoutInfo.bindings.push_back(_lsmBindingInfo);
	_depthBindingInfo.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	_depthBindingInfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthLayoutInfo.bindings.push_back(_depthBindingInfo);
	_depthLayout = renderer.CreateLayout(depthLayoutInfo);

	VkDescriptorType uboTypes[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	_depthDescriptorPool.Construct(imageCount * GetSize(), _depthLayout, uboTypes, 2);
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

	_lsmDescriptorPool.Cleanup();
	_depthDescriptorPool.Cleanup();
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
		/*
		const float oX = _info.shadowResolution.x;
		const float oY = _info.shadowResolution.y;
		glm::mat4 projection = glm::ortho(0.f, oX, 0.f, oY, camera.clipNear, camera.clipFar);
		*/

		//glm::mat4 projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, camera.clipNear, camera.clipFar); // TODO somehow produces corrupt values.
		glm::mat4 projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, .1f, 1000.f);

		Ubo ubo{};
		ubo.lightSpaceMatrix = projection * view;
		renderer.MapMemory(frame.lsmMemory, &ubo, 0, 1);

		for (const auto [shadowCaster, shadowCasterSparseId] : shadowCasters)
		{
			auto& mesh = meshes[shadowCasterSparseId];
			auto& bakedTransform = bakedTransforms[transforms.GetDenseId(shadowCasterSparseId)];

			renderSystem.UseMesh(mesh);
			renderer.BindDescriptorSets(&frame.lsmDescriptor, 1);
			renderer.UpdatePushConstant(_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, bakedTransform);
			renderer.Draw(mesh.indCount);
		}

		renderer.EndRenderPass();
	}

	renderer.EndCommandBufferRecording();
	renderer.Submit(&_commandBuffer, 1, nullptr, nullptr, _fence);
	renderer.WaitForFence(_fence);
}

VkDescriptorSetLayout Light3d::System::GetLayout() const
{
	return _depthLayout;
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

	frame.lsmBuffer = renderer.CreateBuffer<Ubo>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	frame.lsmMemory = renderer.AllocateMemory(frame.lsmBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(frame.lsmBuffer, frame.lsmMemory);

	frame.lsmDescriptor = _lsmDescriptorPool.Get();
	renderer.BindBuffer(frame.lsmDescriptor, frame.lsmBuffer, _lsmBindingInfo, 0, 0);

	frame.depthDescriptor = _depthDescriptorPool.Get();
	frame.depthSampler = renderer.CreateSampler();
	renderer.BindBuffer(frame.depthDescriptor, frame.lsmBuffer, _depthBindingInfo, 0, 0);
	renderer.BindSampler(frame.depthDescriptor, frame.depthBuffer.imageView, frame.depthSampler, 1, 0);
}

void Light3d::System::CleanupInstanceFrame(Frame& frame, Light3d&, const uint32_t)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	_lsmDescriptorPool.Add(frame.lsmDescriptor);
	_depthDescriptorPool.Add(frame.depthDescriptor);

	renderer.DestroySampler(frame.depthSampler);
	renderSystem.DestroyDepthBuffer(frame.depthBuffer);
	renderer.DestroyFrameBuffer(frame.frameBuffer);
	renderer.DestroyBuffer(frame.lsmBuffer);
	renderer.FreeMemory(frame.lsmMemory);
}
