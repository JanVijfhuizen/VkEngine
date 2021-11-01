#include "pch.h"

#include "Cecsar.h"
#include "VkRenderer/WindowSystemGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "FileReader.h"
#include "VkRenderer/PipelineInfo.h"
#include "Vertex.h"
#include "VkRenderer/DescriptorLayoutInfo.h"
#include "VkRenderer/SwapChain.h"
#include "Mesh.h"
#include "RenderSystem.h"
#include "SoASet.h"

// TODO: use material frames and bind material diffuseTexture to active sampler.

struct Transform final
{
	glm::vec2 transPos{};
	glm::vec2 transScale{1};
	float transRot = 0;
};

struct Camera final
{
	glm::vec3 pos;
	float aspectRatio;
};

int main()
{
	ce::Cecsar cecsar{100};

	RenderSystem renderSystem{};

	auto& windowSystem = renderSystem.GetWindowSystem();
	auto& renderer = renderSystem.GetVkRenderer();
	auto& swapChain = renderSystem.GetSwapChain();

	const auto vertCode = FileReader::Read("Shaders/vert.spv");
	const auto fragCode = FileReader::Read("Shaders/frag.spv");

	const auto vertModule = renderer.CreateShaderModule(vertCode);
	const auto fragModule = renderer.CreateShaderModule(fragCode);

	vi::DescriptorLayoutInfo materialLayoutInfo{};
	vi::BindingInfo diffuseBinding{};
	diffuseBinding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	diffuseBinding.flag = VK_SHADER_STAGE_FRAGMENT_BIT;
	materialLayoutInfo.bindings.push_back(diffuseBinding);
	const auto materialLayout = renderer.CreateLayout(materialLayoutInfo);

	vi::DescriptorLayoutInfo camLayoutInfo{};
	vi::BindingInfo camBinding{};
	camBinding.size = sizeof Camera;
	camBinding.flag = VK_SHADER_STAGE_VERTEX_BIT;
	camLayoutInfo.bindings.push_back(camBinding);
	const auto camLayout = renderer.CreateLayout(camLayoutInfo);

	vi::PipelineLayoutInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.push_back(camLayout);
	pipelineInfo.setLayouts.push_back(materialLayout);
	pipelineInfo.modules.push_back(
		{
			vertModule,
			VK_SHADER_STAGE_VERTEX_BIT
		});
	pipelineInfo.modules.push_back(
		{
			fragModule,
			VK_SHADER_STAGE_FRAGMENT_BIT
		});
	pipelineInfo.pushConstants.push_back(
		{
			sizeof Transform,
			VK_SHADER_STAGE_VERTEX_BIT
		});
	pipelineInfo.renderPass = swapChain.GetRenderPass();
	pipelineInfo.extent = swapChain.GetExtent();

	const auto pipeline = renderer.CreatePipeline(pipelineInfo);

	Mesh::Info meshInfo{};
	for (auto& vertex : meshInfo.vertices)
		vertex.pos /= 2;
	const auto mesh = renderSystem.CreateMesh(meshInfo);

	VkDescriptorType uboType[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	const auto uboPool = renderer.CreateDescriptorPool(uboType, 2, swapChain.GetImageCount() * 2);

	VkDescriptorSet sets[2];
	renderer.CreateDescriptorSets(uboPool, camLayout, &sets[0], 1);
	renderer.CreateDescriptorSets(uboPool, materialLayout, &sets[1], 1);

	const auto camBuffer = renderer.CreateBuffer<Camera>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	const auto camMem = renderer.AllocateMemory(camBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(camBuffer, camMem);
	renderer.BindBuffer(sets[0], camBuffer, camBinding, 0, 0);

	const auto texture = renderSystem.CreateTexture("Example.jpg");

	const auto imgSampler = renderer.CreateSampler();

	renderer.BindSampler(sets[1], texture.imageView, imgSampler, 0, 0);

	while(true)
	{
		bool quit;
		renderSystem.BeginFrame(&quit);
		if (quit)
			break;

		renderer.BindPipeline(pipeline);
		renderer.BindDescriptorSets(sets, 2);

		renderSystem.UseMesh(mesh);

		static float f = 0;
		f += 0.001f;
		if (f > 360)
			f = 0;

		Transform transform{};
		transform.transRot = f;

		renderer.UpdatePushConstant(pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, transform);

		const auto resolution = windowSystem.GetVkInfo().resolution;

		// todo every framebuffer now uses the same buffers, so this is wrong. At the same time, this is only used for testing.
		Camera camera{};
		camera.aspectRatio = static_cast<float>(resolution.x) / resolution.y;
		camera.pos.x = f / 10;
		renderer.MapMemory(camMem, &camera, 0, 1);

		renderer.Draw(meshInfo.indices.size());

		renderSystem.EndFrame();
	}

	renderer.DeviceWaitIdle();
	renderer.DestroySampler(imgSampler);

	renderSystem.DestroyMesh(mesh);
	renderSystem.DestroyTexture(texture);

	renderer.FreeMemory(camMem);
	renderer.DestroyBuffer(camBuffer);
	
	renderer.DestroyDescriptorPool(uboPool);

	renderer.DestroyPipeline(pipeline);
	renderer.DestroyLayout(camLayout);
	renderer.DestroyLayout(materialLayout);

	renderer.DestroyShaderModule(vertModule);
	renderer.DestroyShaderModule(fragModule);

	return 0;
}