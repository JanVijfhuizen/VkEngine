#pragma once

namespace vi
{
	struct Queues final
	{
		union
		{
			struct
			{
				// Todo transfer buffer.
				VkQueue graphics;
				VkQueue present;
			};
			VkQueue values[2];
		};
	};
}