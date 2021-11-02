#include "pch.h"
#include "UnlitMaterial.h"
#include "FileReader.h"
#include "Camera.h"
#include "Transform.h"
#include "VkRenderer/PipelineInfo.h"
#include "VkRenderer/DescriptorLayoutInfo.h"
#include "VkRenderer/WindowSystemGLFW.h"

UnlitMaterial::System::System(const uint32_t size) : ShaderSet<UnlitMaterial, Frame>(size)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& swapChain = renderSystem.GetSwapChain();

	auto& cameraSystem = Singleton<Camera::System>::Get();

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

	vi::PipelineLayoutInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.push_back(cameraSystem.GetLayout());
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
	ShaderSet<UnlitMaterial, Frame>::Cleanup();

	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	renderer.DestroyPipeline(_pipeline);
	renderer.DestroyLayout(_materialLayout);
	renderer.DestroyShaderModule(_vertModule);
	renderer.DestroyShaderModule(_fragModule);
	renderer.DestroyDescriptorPool(_uboPool);
}

void UnlitMaterial::System::ConstructInstanceFrame(Frame& frame, UnlitMaterial& material, const uint32_t denseId)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	renderer.CreateDescriptorSets(_uboPool, _materialLayout, &frame.descriptorSet, 1);
	frame.matDiffuseSampler = renderer.CreateSampler();
}

void UnlitMaterial::System::CleanupInstanceFrame(Frame& frame, UnlitMaterial& material, const uint32_t denseId)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	renderer.DestroySampler(frame.matDiffuseSampler);
}

void UnlitMaterial::System::Update()
{
	ShaderSet<UnlitMaterial, Frame>::Update();

	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& swapChain = renderSystem.GetSwapChain();

	auto& cameraSystem = Singleton<Camera::System>::Get();
	auto& frames = GetSets()[swapChain.GetCurrentImageIndex() + 1];

	auto& transforms = Singleton<SparseSet<Transform>>::Get();
	auto& meshes = Singleton<SparseSet<Mesh>>::Get();
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
	cameraSet = cameraSystem.GetCurrentFrameSet().Get<Camera::Frame>(0).descriptor;

	renderer.BindPipeline(_pipeline);

	for (const auto [instance, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);
		auto& frame = frames.Get<Frame>(denseId);
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
