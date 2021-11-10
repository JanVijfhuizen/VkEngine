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

	// Add quad entity.
	Mesh::Info2d meshInfo{};
	for (auto& vertex : meshInfo.vertices)
		vertex.position /= 2;

	const auto quadEntity = cecsar.AddEntity();
	transformSystem->Insert(quadEntity.index);
	auto& mesh = meshSystem->Insert(quadEntity.index);
	mesh = renderSystem.CreateMesh(meshInfo.vertices, meshInfo.indices);
	auto& unlitMaterial = unlitMaterialSystem->Insert(quadEntity.index);
	unlitMaterial.diffuseTexture = &texture;

	// Add cube entity.
	std::vector<Vertex> cubeVerts{};
	std::vector<int8_t> cubeInds{};
	Mesh::System::Load("Cube.obj", cubeVerts, cubeInds);

	const auto cubeEntity = cecsar.AddEntity();
	transformSystem->Insert(cubeEntity.index);
	auto& cubeMesh = meshSystem->Insert(cubeEntity.index);
	cubeMesh = renderSystem.CreateMesh<Vertex, int8_t>(cubeVerts, cubeInds);

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
	renderSystem.DestroyMesh(cubeMesh);
	renderSystem.DestroyTexture(texture);

	cameraSystem->Cleanup();
	delete transformSystem;
	delete cameraSystem;
	delete meshSystem;
	unlitMaterialSystem->Cleanup();
	delete unlitMaterialSystem;
	return 0;
}
