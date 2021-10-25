#include "pch.h"

#include "Cecsar.h"
#include "VkRenderer/WindowSystemGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "FileReader.h"
#include "VkRenderer/PipelineInfo.h"
#include "Vertex.h"
#include "VkRenderer/RenderPassInfo.h"
#include "VkRenderer/DescriptorLayoutInfo.h"
#include "VkRenderer/SwapChain.h"
#include "Mesh.h"

struct Transform final
{
	glm::vec2 transPos;
	glm::vec2 transScale;
	float transRot;
};

struct Camera final
{
	glm::vec3 pos;
	float aspectRatio;
};

int main()
{
	vi::WindowSystemGLFW windowSystem{};

	vi::VkRenderer::Settings settings;
	settings.debugger.validationLayers.push_back("VK_LAYER_KHRONOS_validation");

	vi::VkRenderer renderer
	{
		windowSystem,
		settings
	};

	vi::SwapChain swapChain{ renderer };

	vi::RenderPassInfo renderPassInfo{};
	vi::RenderPassInfo::Attachment renderPassAttachment{};
	renderPassInfo.attachments.push_back(renderPassAttachment);
	renderPassInfo.format = swapChain.format;

	const auto renderPass = renderer.CreateRenderPass(renderPassInfo);
	swapChain.SetRenderPass(renderPass);

	const auto vertCode = FileReader::Read("Shaders/vert.spv");
	const auto fragCode = FileReader::Read("Shaders/frag.spv");

	const auto vertModule = renderer.CreateShaderModule(vertCode);
	const auto fragModule = renderer.CreateShaderModule(fragCode);

	vi::DescriptorLayoutInfo camLayoutInfo{};
	vi::DescriptorLayoutInfo::Binding camBinding{};
	camBinding.size = sizeof Camera;
	camBinding.flag = VK_SHADER_STAGE_VERTEX_BIT;
	camLayoutInfo.bindings.push_back(camBinding);
	const auto camLayout = renderer.CreateLayout(camLayoutInfo);

	vi::PipelineLayoutInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	//pipelineInfo.setLayouts.push_back(camLayout);
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
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.extent = swapChain.extent;

	const auto pipeline = renderer.CreatePipeline(pipelineInfo);

	vk::Mesh::Info meshInfo{};
	for (auto& vertex : meshInfo.vertices)
	{
		vertex.pos /= 2;
	}

	const auto vertBuffer = renderer.CreateBuffer<Vertex>(meshInfo.vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	const auto vertMem = renderer.AllocateMemory(vertBuffer);
	renderer.BindMemory(vertBuffer, vertMem);
	renderer.MapMemory(vertMem, meshInfo.vertices.data(), 0, meshInfo.vertices.size());

	const auto indBuffer = renderer.CreateBuffer<uint16_t>(meshInfo.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	const auto indMem = renderer.AllocateMemory(indBuffer);
	renderer.BindMemory(indBuffer, indMem);
	renderer.MapMemory(indMem, meshInfo.indices.data(), 0, meshInfo.indices.size());

	while(true)
	{
		bool quit;
		windowSystem.BeginFrame(quit);
		if (quit)
			break;

		vi::SwapChain::Buffers* buffers;
		vi::SwapChain::Frame* frame;

		swapChain.GetNext(buffers, frame);

		auto& extent = swapChain.extent;

		renderer.BeginCommandBufferRecording(buffers->commandBuffer);
		renderer.BeginRenderPass(buffers->commandBuffer, buffers->frameBuffer, swapChain.renderPass, {}, { extent.width, extent.height});

		renderer.BindPipeline(buffers->commandBuffer, pipeline.pipeline);

		//renderer.BindDescriptorSets(image->commandBuffer, pipeline.layout, &camLayout, 1);
		renderer.BindVertexBuffer(buffers->commandBuffer, vertBuffer);
		renderer.BindIndicesBuffer(buffers->commandBuffer, indBuffer);

		Transform transform{};

		renderer.UpdatePushConstant(buffers->commandBuffer, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, transform);

		renderer.Draw(buffers->commandBuffer, meshInfo.indices.size());

		renderer.EndRenderPass(buffers->commandBuffer);
		renderer.EndCommandBufferRecording(buffers->commandBuffer);

		swapChain.WaitForImage();
		renderer.Submit(&buffers->commandBuffer, 1, frame->imageAvailableSemaphore, frame->renderFinishedSemaphore, frame->inFlightFence);
		const auto result = swapChain.Present();
		if (!result)
		{
			// Recreate pipeline.
		}
	}

	renderer.DestroyPipeline(pipeline);
	renderer.DestroyLayout(camLayout);
	renderer.DestroyRenderPass(renderPass);

	renderer.FreeMemory(vertMem);
	renderer.FreeMemory(indMem);

	renderer.DestroyBuffer(vertBuffer);
	renderer.DestroyBuffer(indBuffer);

	renderer.DestroyShaderModule(vertModule);
	renderer.DestroyShaderModule(fragModule);

	return 0;
}