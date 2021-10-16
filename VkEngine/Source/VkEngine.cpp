#include "pch.h"
#include "Cecsar.h"
#include "VkRenderer/WindowSystemGLFW.h"
#include "VkRenderer/VkRenderer.h"

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
	vi::VkRenderer renderer
	{
		windowSystem
	};

	while(true)
	{
		bool quit;
		windowSystem.BeginFrame(quit);
		if (quit)
			break;
	}

	return 0;
}
