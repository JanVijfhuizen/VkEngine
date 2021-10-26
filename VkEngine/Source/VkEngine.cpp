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
	settings.debugger.validationLayers.push_back("VK_LAYER_RENDERDOC_Capture");

	vi::VkRenderer renderer{};
	renderer.Construct(windowSystem, settings);

	vi::SwapChain swapChain{};
	swapChain.Construct(renderer);

	vi::RenderPassInfo renderPassInfo{};
	vi::RenderPassInfo::Attachment renderPassAttachment{};
	renderPassInfo.attachments.push_back(renderPassAttachment);
	renderPassInfo.format = swapChain.GetFormat();

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
	pipelineInfo.extent = swapChain.GetExtent();

	const auto pipeline = renderer.CreatePipeline(pipelineInfo);

	vk::Mesh::Info meshInfo{};
	for (auto& vertex : meshInfo.vertices)
		vertex.pos /= 2;

	const auto vertStagingBuffer = renderer.CreateBuffer<Vertex>(meshInfo.vertices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto vertStagingMem = renderer.AllocateMemory(vertStagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(vertStagingBuffer, vertStagingMem);
	renderer.MapMemory(vertStagingMem, meshInfo.vertices.data(), 0, meshInfo.vertices.size());

	const auto vertBuffer = renderer.CreateBuffer<Vertex>(meshInfo.vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	const auto vertMem = renderer.AllocateMemory(vertBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	renderer.BindMemory(vertBuffer, vertMem);

	auto cpyCommandBuffer = renderer.CreateCommandBuffer();
	const auto cpyFence = renderer.CreateFence();

	renderer.BeginCommandBufferRecording(cpyCommandBuffer);
	renderer.CopyBuffer(vertStagingBuffer, vertBuffer, meshInfo.vertices.size() * sizeof(Vertex));
	renderer.EndCommandBufferRecording();

	renderer.Submit(&cpyCommandBuffer, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, cpyFence);
	renderer.WaitForFence(cpyFence);
	renderer.DestroyFence(cpyFence);
	renderer.DestroyCommandBuffer(cpyCommandBuffer);

	renderer.DestroyBuffer(vertStagingBuffer);
	renderer.FreeMemory(vertStagingMem);

	const auto indBuffer = renderer.CreateBuffer<uint16_t>(meshInfo.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	const auto indMem = renderer.AllocateMemory(indBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(indBuffer, indMem);
	renderer.MapMemory(indMem, meshInfo.indices.data(), 0, meshInfo.indices.size());

	while(true)
	{
		bool quit;
		windowSystem.BeginFrame(quit);
		if (quit)
			break;

		vi::SwapChain::Image image;
		vi::SwapChain::Frame frame;
		swapChain.GetNext(image, frame);

		const auto extent = swapChain.GetExtent();

		renderer.BeginCommandBufferRecording(image.commandBuffer);
		renderer.BeginRenderPass(image.frameBuffer, swapChain.GetRenderPass(), {}, { extent.width, extent.height});

		renderer.BindPipeline(pipeline.pipeline);

		//renderer.BindDescriptorSets(image->commandBuffer, pipeline.layout, &camLayout, 1);
		renderer.BindVertexBuffer(vertBuffer);
		renderer.BindIndicesBuffer(indBuffer);

		Transform transform{};

		renderer.UpdatePushConstant(pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, transform);

		renderer.Draw(meshInfo.indices.size());

		renderer.EndRenderPass();
		renderer.EndCommandBufferRecording();

		renderer.Submit(&image.commandBuffer, 1, frame.imageAvailableSemaphore, frame.renderFinishedSemaphore, frame.inFlightFence);
		const auto result = swapChain.Present();
		if (!result)
		{
			// Recreate pipeline.
		}
	}

	swapChain.Cleanup();

	renderer.DestroyPipeline(pipeline);
	renderer.DestroyLayout(camLayout);
	renderer.DestroyRenderPass(renderPass);

	renderer.FreeMemory(vertMem);
	renderer.FreeMemory(indMem);

	renderer.DestroyBuffer(vertBuffer);
	renderer.DestroyBuffer(indBuffer);

	renderer.DestroyShaderModule(vertModule);
	renderer.DestroyShaderModule(fragModule);

	renderer.Cleanup();

	return 0;
}