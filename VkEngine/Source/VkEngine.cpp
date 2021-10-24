#include "pch.h"
#include "Cecsar.h"
#include "VkRenderer/WindowSystemGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "FileReader.h"
#include "VkRenderer/PipelineInfo.h"
#include "Vertex.h"
#include "VkRenderer/RenderPassInfo.h"
#include "VkRenderer/DescriptorLayoutInfo.h"

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

	const auto vertCode = FileReader::Read("Shaders/vert.spv");
	const auto fragCode = FileReader::Read("Shaders/frag.spv");

	const auto vertModule = renderer.CreateShaderModule(vertCode);
	const auto fragModule = renderer.CreateShaderModule(fragCode);

	vi::RenderPassInfo renderPassInfo{};
	vi::RenderPassInfo::Attachment renderPassAttachment{};
	renderPassInfo.attachments.push_back(renderPassAttachment);

	const auto renderPass = renderer.CreateRenderPass(renderPassInfo);

	vi::DescriptorLayoutInfo camLayoutInfo{};
	vi::DescriptorLayoutInfo::Binding camBinding{};
	camBinding.size = sizeof Camera;
	camBinding.flag = VK_SHADER_STAGE_VERTEX_BIT;
	camLayoutInfo.bindings.push_back(camBinding);
	const auto camLayout = renderer.CreateLayout(camLayoutInfo);

	vi::PipelineLayout pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
	pipelineInfo.setLayouts.push_back(camLayout);
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

	const auto pipeline = renderer.CreatePipeline(pipelineInfo);
	renderer.swapChain.SetRenderPass(renderPass);

	while(true)
	{
		bool quit;
		windowSystem.BeginFrame(quit);
		if (quit)
			break;

		auto& swapChain = renderer.swapChain;

		vi::SwapChain::Image* image;
		vi::SwapChain::Frame* frame;

		swapChain.GetNext(image, frame);
		auto& extent = swapChain.extent;

		renderer.BeginCommandBufferRecording(frame->commandBuffer);
		renderer.BeginRenderPass(frame->commandBuffer, image->frameBuffer, swapChain.renderPass, {}, { extent.width, extent.height});
		renderer.EndRenderPass(frame->commandBuffer, swapChain.renderPass);
		renderer.EndCommandBufferRecording(frame->commandBuffer);

		auto submitInfo = renderer.Submit(&frame->commandBuffer, 1, frame->imageAvailableSemaphore, frame->renderFinishedSemaphore);
		renderer.swapChain.Present(submitInfo);
	}

	renderer.DestroyPipeline(pipeline);
	renderer.DestroyLayout(camLayout);
	renderer.DestroyRenderPass(renderPass);

	renderer.DestroyShaderModule(vertModule);
	renderer.DestroyShaderModule(fragModule);

	return 0;
}
