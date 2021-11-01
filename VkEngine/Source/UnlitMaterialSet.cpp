#include "pch.h"
#include "UnlitMaterialSet.h"
#include "FileReader.h"
#include "Camera.h"
#include "Transform.h"
#include "VkRenderer/PipelineInfo.h"
#include "VkRenderer/DescriptorLayoutInfo.h"

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
	vi::BindingInfo camBinding{};
	camBinding.size = sizeof Camera;
	camBinding.flag = VK_SHADER_STAGE_VERTEX_BIT;
	camLayoutInfo.bindings.push_back(camBinding);
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
}

void UnlitMaterial::System::ConstructInstance(const uint32_t denseId)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
}

void UnlitMaterial::System::CleanupInstance(const uint32_t denseId)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
}
