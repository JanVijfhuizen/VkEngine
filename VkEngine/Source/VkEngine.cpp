#include "pch.h"
#include "Cecsar.h"
#include "VkRenderer/WindowSystemGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "Vertex2d.h"
#include "Mesh.h"
#include "RenderSystem.h"
#include "Singleton.h"
#include "Transform.h"
#include "Camera.h"
#include "UnlitMaterial2d.h"

int main()
{
	const uint32_t entityCount = 100;

	ce::Cecsar cecsar{entityCount};
	RenderSystem renderSystem{};
	RenderSystem::Instance::Set(&renderSystem);

	auto& renderer = renderSystem.GetVkRenderer();

	const auto transformSystem = new Transform::System(entityCount);
	Transform::System::Instance::Set(transformSystem);
	cecsar.AddSet(transformSystem);
	
	const auto meshSystem = new Mesh::System(entityCount);
	Mesh::System::Instance::Set(meshSystem);
	cecsar.AddSet(meshSystem);

	const auto cameraSystem = new Camera::System(entityCount);
	Camera::System::Instance::Set(cameraSystem);
	cecsar.AddSet(cameraSystem);

	const auto unlitMaterialSystem = new UnlitMaterial2d::System(entityCount);
	UnlitMaterial2d::System::Instance::Set(unlitMaterialSystem);
	cecsar.AddSet(unlitMaterialSystem);

	// Create scene instances.
	const auto camEntity = cecsar.AddEntity();
	cameraSystem->Insert(camEntity.id);
	transformSystem->Insert(camEntity.id);

	auto texture = renderSystem.CreateTexture("Example.jpg");

	Mesh::Info meshInfo{};
	for (auto& vertex : meshInfo.vertices)
		vertex.position /= 2;

	const auto quadEntity = cecsar.AddEntity();
	transformSystem->Insert(quadEntity.id);
	auto& mesh = meshSystem->Insert(quadEntity.id);
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

	cameraSystem->Cleanup();
	delete transformSystem;
	delete cameraSystem;
	delete meshSystem;
	unlitMaterialSystem->Cleanup();
	delete unlitMaterialSystem;
	return 0;
}
