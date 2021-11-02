#include "pch.h"
#include "Cecsar.h"
#include "VkRenderer/WindowSystemGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "Vertex.h"
#include "Mesh.h"
#include "RenderSystem.h"
#include "Singleton.h"
#include "Transform.h"
#include "Camera.h"
#include "UnlitMaterial.h"

// Todo pool descriptor sets.

int main()
{
	const uint32_t entityCount = 100;

	ce::Cecsar cecsar{entityCount};
	RenderSystem renderSystem{};

	auto& renderer = renderSystem.GetVkRenderer();

	ce::SparseSet<Transform> transforms{entityCount};
	Singleton<ce::SparseSet<Transform>>::Set(&transforms);
	cecsar.AddSet(&transforms);
	
	ce::SparseSet<Mesh> meshes{entityCount};
	Singleton<ce::SparseSet<Mesh>>::Set(&meshes);
	cecsar.AddSet(&meshes);

	const auto cameraSystem = new Camera::System(entityCount);
	Singleton<Camera::System>::Set(cameraSystem);
	cecsar.AddSet(cameraSystem);

	const auto unlitMaterialSystem = new UnlitMaterial::System(entityCount);
	Singleton<UnlitMaterial::System>::Set(unlitMaterialSystem);
	cecsar.AddSet(unlitMaterialSystem);

	// Create scene instances.
	const auto camEntity = cecsar.AddEntity();
	cameraSystem->Insert(camEntity.id);
	transforms.Insert(camEntity.id);

	auto texture = renderSystem.CreateTexture("Example.jpg");

	Mesh::Info meshInfo{};
	for (auto& vertex : meshInfo.vertices)
		vertex.pos /= 2;

	const auto quadEntity = cecsar.AddEntity();
	transforms.Insert(quadEntity.id);
	auto& mesh = meshes.Insert(quadEntity.id);
	mesh = renderSystem.CreateMesh(meshInfo);
	auto& unlitMaterial = unlitMaterialSystem->Insert(quadEntity.id);
	unlitMaterial.diffuseTexture = &texture;

	while(true)
	{
		bool quit;
		renderSystem.BeginFrame(&quit);
		if (quit)
			break;

		cameraSystem->Update();
		unlitMaterialSystem->Update();

		renderSystem.EndFrame();
	}

	renderer.DeviceWaitIdle();

	renderSystem.DestroyMesh(mesh);
	renderSystem.DestroyTexture(texture);

	delete cameraSystem;
	delete unlitMaterialSystem;
	return 0;
}
