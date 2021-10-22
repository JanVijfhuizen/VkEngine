#include "pch.h"
#include "Cecsar.h"
#include "VkRenderer/WindowSystemGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "FileReader.h"
#include "VkRenderer/PipelineLayoutInfo.h"
#include "Vertex.h"

struct Transform final
{
	float x, y, z;
};

int main()
{
	ce::Cecsar<100> cecsar;
	ce::SparseSet<Transform, 100> transforms;

	cecsar.AddSet(&transforms);
	const auto entity = cecsar.AddEntity();

	auto& transform = transforms.Insert(entity.index);

	transforms[12] = {};

	for (const auto [instance, index] : transforms)
	{
		
	}

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

	vi::PipelineLayoutInfo pipelineInfo{};
	pipelineInfo.attributeDescriptions = Vertex::GetAttributeDescriptions();
	pipelineInfo.bindingDescription = Vertex::GetBindingDescription();
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

	const auto pipeline = renderer.CreatePipelineLayout(pipelineInfo);

	while(true)
	{
		bool quit;
		windowSystem.BeginFrame(quit);
		if (quit)
			break;
	}

	renderer.DestroyShaderModule(vertModule);
	renderer.DestroyShaderModule(fragModule);

	renderer.DestroyPipelineLayout(pipeline);

	return 0;
}
