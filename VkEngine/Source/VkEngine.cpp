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
#include "TextureLoader.h"
#include "RenderSystem.h"

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

	VkDescriptorType uboType[] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
	const auto uboPool = renderer.CreateDescriptorPool(uboType, 2, swapChain.GetImageCount() * 2);

	VkDescriptorSet sets[2];
	renderer.CreateDescriptorSets(uboPool, camLayout, &sets[0], 1);
	renderer.CreateDescriptorSets(uboPool, materialLayout, &sets[1], 1);

	const auto camBuffer = renderer.CreateBuffer<Camera>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	const auto camMem = renderer.AllocateMemory(camBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(camBuffer, camMem);
	renderer.BindBuffer(sets[0], camBuffer, camBinding, 0, 0);

	int32_t w, h, d;
	const auto texture = TextureLoader::Load("Textures/Example.jpg", w, h, d);
	const auto texStagingBuffer = renderer.CreateBuffer<unsigned char>(w * h * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	const auto texStagingMem = renderer.AllocateMemory(texStagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	renderer.BindMemory(texStagingBuffer, texStagingMem);
	renderer.MapMemory(texStagingMem, texture, 0, w * h * 4);
	TextureLoader::Free(texture);

	const auto img = renderer.CreateImage({ w, h });
	const auto imgMem = renderer.AllocateMemory(img, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	renderer.BindMemory(img, imgMem);

	auto imgCmd = renderer.CreateCommandBuffer();
	renderer.BeginCommandBufferRecording(imgCmd);
	renderer.TransitionImageLayout(img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	renderer.EndCommandBufferRecording();

	const auto imgFence = renderer.CreateFence();
	renderer.Submit(&imgCmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, imgFence);
	renderer.WaitForFence(imgFence);

	renderer.BeginCommandBufferRecording(imgCmd);
	renderer.CopyBuffer(texStagingBuffer, img, w, h);
	renderer.EndCommandBufferRecording();
	renderer.Submit(&imgCmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, imgFence);
	renderer.WaitForFence(imgFence);

	renderer.BeginCommandBufferRecording(imgCmd);
	renderer.TransitionImageLayout(img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	renderer.EndCommandBufferRecording();
	renderer.Submit(&imgCmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, imgFence);
	renderer.WaitForFence(imgFence);

	renderer.DestroyFence(imgFence);
	renderer.DestroyCommandBuffer(imgCmd);
	renderer.DestroyBuffer(texStagingBuffer);
	renderer.FreeMemory(texStagingMem);

	const auto imgView = renderer.CreateImageView(img);
	const auto imgSampler = renderer.CreateSampler();
	renderer.BindSampler(sets[1], imgView, imgSampler, 0, 0);

	while(true)
	{
		bool quit;
		renderSystem.BeginFrame(&quit);
		if (quit)
			break;

		renderer.BindPipeline(pipeline);
		renderer.BindDescriptorSets(sets, 2);

		renderer.BindVertexBuffer(vertBuffer);
		renderer.BindIndicesBuffer(indBuffer);

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

	swapChain.Cleanup();

	renderer.DestroySampler(imgSampler);
	renderer.FreeMemory(imgMem);
	renderer.DestroyImageView(imgView);
	renderer.DestroyImage(img);

	renderer.FreeMemory(camMem);
	renderer.DestroyBuffer(camBuffer);
	
	renderer.DestroyDescriptorPool(uboPool);

	renderer.DestroyPipeline(pipeline);
	renderer.DestroyLayout(camLayout);

	renderer.FreeMemory(vertMem);
	renderer.FreeMemory(indMem);

	renderer.DestroyBuffer(vertBuffer);
	renderer.DestroyBuffer(indBuffer);

	renderer.DestroyShaderModule(vertModule);
	renderer.DestroyShaderModule(fragModule);

	renderer.Cleanup();

	return 0;
}