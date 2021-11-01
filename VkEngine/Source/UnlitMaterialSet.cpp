﻿#include "pch.h"
#include "UnlitMaterialSet.h"
#include "FileReader.h"
#include "Camera.h"
#include "Transform.h"
#include "VkRenderer/PipelineInfo.h"
#include "VkRenderer/DescriptorLayoutInfo.h"
#include "VkRenderer/WindowSystemGLFW.h"

UnlitMaterial::System::System(const uint32_t size) : MaterialSet<UnlitMaterial, Frame>(size)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& swapChain = renderSystem.GetSwapChain();

	const auto vertCode = FileReader::Read("Shaders/vert.spv");
	const auto fragCode = FileReader::Read("Shaders/frag.spv");

	_vertModule = renderer.CreateShaderModule(vertCode);
	_fragModule = renderer.CreateShaderModule(fragCode);

	vi::DescriptorLayoutInfo materialLayoutInfo{};
	vi::BindingInfo diffuseBinding{};
	diffuseBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	diffuseBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	materialLayoutInfo.bindings.push_back(diffuseBinding);
	_materialLayout = renderer.CreateLayout(materialLayoutInfo);

	vi::DescriptorLayoutInfo camLayoutInfo{};
	_camBinding.size = sizeof Camera;
	_camBinding.flag = VK_SHADER_STAGE_VERTEX_BIT;
	camLayoutInfo.bindings.push_back(_camBinding);
	_camLayout = renderer.CreateLayout(camLayoutInfo);

	vi::PipelineLayoutInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.push_back(_camLayout);
	pipelineInfo.setLayouts.push_back(_materialLayout);
	pipelineInfo.modules.push_back(
		{
			_vertModule,
			VK_SHADER_STAGE_VERTEX_BIT
		});
	pipelineInfo.modules.push_back(
		{
			_fragModule,
			VK_SHADER_STAGE_FRAGMENT_BIT
		});
	pipelineInfo.pushConstants.push_back(
		{
			sizeof Transform,
			VK_SHADER_STAGE_VERTEX_BIT
		});
	pipelineInfo.renderPass = swapChain.GetRenderPass();
	pipelineInfo.extent = swapChain.GetExtent();

	_pipeline = renderer.CreatePipeline(pipelineInfo);

	const uint32_t imageCount = swapChain.GetImageCount();
	VkDescriptorType uboType[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	_uboPool = renderer.CreateDescriptorPool(uboType, 2, imageCount * GetSize());
}

void UnlitMaterial::System::Cleanup()
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	renderer.DestroyPipeline(_pipeline);
	renderer.DestroyLayout(_camLayout);
	renderer.DestroyLayout(_materialLayout);
	renderer.DestroyShaderModule(_vertModule);
	renderer.DestroyShaderModule(_fragModule);
	renderer.DestroyDescriptorPool(_uboPool);
}

void UnlitMaterial::System::ConstructInstance(const uint32_t denseId)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	auto& sets = GetSets();

	for (uint32_t i = 1; i < sets.size(); ++i)
	{
		auto& set = sets[i];
		auto& frame = set.Get<Frame>(denseId);

		renderer.CreateDescriptorSets(_uboPool, _camLayout, &frame.camSet, 1);
		renderer.CreateDescriptorSets(_uboPool, _materialLayout, &frame.matSet, 1);

		// Todo bind camera buffer and create camera system.

		frame.matDiffuseSampler = renderer.CreateSampler();
	}
}

void UnlitMaterial::System::CleanupInstance(const uint32_t denseId)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	auto& sets = GetSets();

	for (uint32_t i = 1; i < sets.size(); ++i)
	{
		auto& set = sets[i];
		auto& frame = set.Get<Frame>(denseId);

		renderer.DestroySampler(frame.matDiffuseSampler);
	}
}

void UnlitMaterial::System::Update()
{
	MaterialSet<UnlitMaterial, Frame>::Update();

	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& swapChain = renderSystem.GetSwapChain();

	auto& frames = GetSets()[swapChain.GetImageCount() + 1];
	auto& transforms = Singleton<SparseSet<Transform>>::Get();
	auto& meshes = Singleton<SparseSet<Mesh>>::Get();

	renderer.BindPipeline(_pipeline);

	for (const auto [instance, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);
		auto& frame = frames.Get<Frame>(denseId);
		auto& mesh = meshes[sparseId];

		renderer.BindDescriptorSets(frame.sets, 2);
		renderSystem.UseMesh(mesh);

		const auto& diffuseTex = *instance.diffuseTexture;
		renderer.BindSampler(frame.matSet, diffuseTex.imageView, frame.matDiffuseSampler, 0, 0);

		auto& transform = transforms[sparseId];
		renderer.UpdatePushConstant(_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, transform);

		renderer.Draw(mesh.indCount);
	}
}
