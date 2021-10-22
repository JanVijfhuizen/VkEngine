#pragma once

namespace vi
{
	struct DescriptorLayoutInfo final
	{
		struct Binding final
		{
			// Todo Textures.
			size_t size = sizeof(int32_t);
			VkShaderStageFlagBits flag;
		};

		std::vector<Binding> bindings{};
	};
}