#pragma once

namespace vi
{
	struct Queues final
	{
		union
		{
			struct
			{
				VkQueue graphics;
				VkQueue present;
			};
			VkQueue values[2];
		};
	};
}