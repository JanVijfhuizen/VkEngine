#include "pch.h"
#include "Light3d.h"
#include "VkRenderer/RenderPassInfo.h"
#include "FileReader.h"
#include "Camera3d.h"
#include "VkRenderer/PipelineInfo.h"
#include "Transform3d.h"
#include "ShadowCaster.h"

Light3d::System::System(const uint32_t size, const Info& info) : ShaderSet<Light3d, Frame>(size), _info(info)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	vi::RenderPassInfo renderPassInfo{};
	renderPassInfo.useDepthAttachment = true;
	_renderPass = renderer.CreateRenderPass(renderPassInfo);

	auto& cameraSystem = Camera3d::System::Instance::Get();

	const auto vertCode = FileReader::Read("Shaders/vert3dShadow.spv");

	_vertModule = renderer.CreateShaderModule(vertCode);

	const vi::DescriptorLayoutInfo materialLayoutInfo{};
	auto layout = renderer.CreateLayout(materialLayoutInfo);

	vi::PipelineLayoutInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex3d::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex3d::GetBindingDescription();
	pipelineInfo.setLayouts.push_back(cameraSystem.GetLayout());
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
}

void Light3d::System::Update()
{
	ShaderSet<Light3d, Frame>::Update();

	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	const auto frames = GetCurrentFrameSet().Get<Frame>();
	auto& cameraSystem = Camera3d::System::Instance::Get();
	auto& shadowCasters = ShadowCaster::System::Instance::Get();
	auto& meshes = Mesh::System::Instance::Get();
	auto& transforms = Transform3d::System::Instance::Get();
	const auto bakedTransforms = transforms.GetSets()[0].Get<Transform3d::Baked>();
	auto cameraSet = cameraSystem.GetCurrentFrameSet().Get<CameraFrame>(0).descriptor;

	VkClearValue clearValue{};
	clearValue.depthStencil = { 1, 0 };

	renderer.BeginCommandBufferRecording(_commandBuffer);
	renderer.BindPipeline(_pipeline);

	for (const auto [light, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);
		auto& frame = frames[denseId];

		renderer.BeginRenderPass(frame.frameBuffer, _renderPass, {}, _info.shadowResolution, &clearValue, 1);

		for (const auto [shadowCaster, shadowCasterSparseId] : shadowCasters)
		{
			auto& mesh = meshes[shadowCasterSparseId];
			auto& bakedTransform = bakedTransforms[transforms.GetDenseId(shadowCasterSparseId)];

			renderSystem.UseMesh(mesh);
			renderer.BindDescriptorSets(&cameraSet, 1);
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
}

void Light3d::System::CleanupInstanceFrame(Frame& frame, Light3d&, const uint32_t)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	renderSystem.DestroyDepthBuffer(frame.depthBuffer);
	renderer.DestroyFrameBuffer(frame.frameBuffer);
}
