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

		struct PushConstant final
		{
			size_t size;
			VkShaderStageFlags flag;
		};

		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		std::vector<Module> modules{};

		std::vector<VkDescriptorSetLayout> setLayouts{};
		std::vector<PushConstant> pushConstants{};

		VkRenderPass renderPass;
		VkExtent2D extent;

		VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkBool32 primitiveRestartEnable = VK_FALSE;
		VkBool32 depthClampEnable = VK_FALSE;
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
		float lineWidth = 1;

		VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
		VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		VkPipeline basePipeline = VK_NULL_HANDLE;
		int32_t basePipelineIndex = -1;
	};
}