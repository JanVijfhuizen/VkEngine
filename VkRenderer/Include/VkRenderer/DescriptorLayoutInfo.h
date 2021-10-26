#pragma once
#include "BindingInfo.h"

namespace vi
{
	struct DescriptorLayoutInfo final
	{
		std::vector<BindingInfo> bindings{};
	};
}
