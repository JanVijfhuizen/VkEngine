#pragma once

namespace vi
{
	struct BindingInfo final
	{
		VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		size_t size = sizeof(int32_t);
		uint32_t count = 1;
		VkShaderStageFlagBits flag;
	};
}