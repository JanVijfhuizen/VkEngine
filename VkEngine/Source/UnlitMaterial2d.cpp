#include "pch.h"
#include "UnlitMaterial2d.h"
#include "FileReader.h"
#include "Camera2d.h"
#include "Transform2d.h"
#include "VkRenderer/PipelineInfo.h"
#include "VkRenderer/DescriptorLayoutInfo.h"
#include "VkRenderer/WindowSystemGLFW.h"

UnlitMaterial2d::System::System(const uint32_t size) : ShaderSet<UnlitMaterial2d, Frame>(size)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& swapChain = renderSystem.GetSwapChain();

	auto& cameraSystem = Camera2d::System::Instance::Get();

	const auto vertCode = FileReader::Read("Shaders/vert2d.spv");
	const auto fragCode = FileReader::Read("Shaders/frag2d.spv");

	_vertModule = renderer.CreateShaderModule(vertCode);
	_fragModule = renderer.CreateShaderModule(fragCode);

	vi::DescriptorLayoutInfo materialLayoutInfo{};
	vi::BindingInfo diffuseBinding{};
	diffuseBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	diffuseBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	materialLayoutInfo.bindings.push_back(diffuseBinding);
	auto layout = renderer.CreateLayout(materialLayoutInfo);

	vi::PipelineLayoutInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex2d::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex2d::GetBindingDescription();
	pipelineInfo.setLayouts.push_back(cameraSystem.GetLayout());
	pipelineInfo.setLayouts.push_back(layout);
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
			sizeof Transform2d,
			VK_SHADER_STAGE_VERTEX_BIT
		});
	pipelineInfo.renderPass = swapChain.GetRenderPass();
	pipelineInfo.extent = swapChain.GetExtent();

	_pipeline = renderer.CreatePipeline(pipelineInfo);

	const uint32_t imageCount = swapChain.GetImageCount();
	VkDescriptorType uboTypes[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	_descriptorPool.Construct(imageCount * GetSize(), layout, uboTypes, 2);
}

void UnlitMaterial2d::System::Cleanup()
{
	ShaderSet<UnlitMaterial2d, Frame>::Cleanup();

	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	renderer.DestroyPipeline(_pipeline);
	renderer.DestroyShaderModule(_vertModule);
	renderer.DestroyShaderModule(_fragModule);
	_descriptorPool.Cleanup();
}

void UnlitMaterial2d::System::ConstructInstanceFrame(Frame& frame, UnlitMaterial2d&, const uint32_t)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	frame.descriptorSet = _descriptorPool.Get();
	frame.matDiffuseSampler = renderer.CreateSampler();
}

void UnlitMaterial2d::System::CleanupInstanceFrame(Frame& frame, UnlitMaterial2d&, const uint32_t)
{
	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	_descriptorPool.Add(frame.descriptorSet);
	renderer.DestroySampler(frame.matDiffuseSampler);
}

void UnlitMaterial2d::System::Update()
{
	ShaderSet<UnlitMaterial2d, Frame>::Update();

	auto& renderSystem = RenderSystem::Instance::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& swapChain = renderSystem.GetSwapChain();

	auto& cameraSystem = Camera2d::System::Instance::Get();
	const auto frames = GetSets()[swapChain.GetCurrentImageIndex() + 1].Get<Frame>();

	auto& transforms = Transform2d::System::Instance::Get();
	auto& meshes = Mesh::System::Instance::Get();
	if (cameraSystem.GetSize() == 0)
		return;

	union
	{
		struct
		{
			VkDescriptorSet cameraSet;
			VkDescriptorSet materialSet;
		};
		VkDescriptorSet sets[2];
	};
	cameraSet = cameraSystem.GetCurrentFrameSet().Get<CameraFrame>(0).descriptor;

	renderer.BindPipeline(_pipeline);

	for (const auto [instance, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);
		auto& frame = frames[denseId];
		auto& mesh = meshes[sparseId];
		auto& transform = transforms[sparseId];
		const auto& diffuseTex = *instance.diffuseTexture;

		materialSet = frame.descriptorSet;

		renderSystem.UseMesh(mesh);
		renderer.BindDescriptorSets(sets, 2);
		renderer.BindSampler(frame.descriptorSet, diffuseTex.imageView, frame.matDiffuseSampler, 0, 0);
		renderer.UpdatePushConstant(_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, transform);
		renderer.Draw(mesh.indCount);
	}
}
