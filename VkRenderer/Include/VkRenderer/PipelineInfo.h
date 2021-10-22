#pragma once

namespace vi
{
	struct PipelineInfo final
	{
		struct Module final
		{
			VkShaderModule module;
			VkShaderStageFlagBits flags;
		};

		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		std::vector<Module> modules{};
	};
}