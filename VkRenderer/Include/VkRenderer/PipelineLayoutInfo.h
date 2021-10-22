#pragma once

namespace vi
{
	struct PipelineLayoutInfo final
	{
		struct Module final
		{
			VkShaderModule module;
			VkShaderStageFlagBits flags;
		};

		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		std::vector<Module> modules{};

		VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkBool32 primitiveRestartEnable = VK_FALSE;
		VkBool32 depthClampEnable = VK_FALSE;
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
		float lineWidth = 1;

		VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
		VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;

		VkPipelineColorBlendAttachmentState colorBlending;
		bool colorBlendingEnabled = false;
	};
}