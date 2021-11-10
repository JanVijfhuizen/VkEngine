#include "pch.h"
#include "Cecsar.h"
#include "VkRenderer/WindowSystemGLFW.h"
#include "VkRenderer/VkRenderer.h"
#include "Vertex2d.h"
#include "Mesh.h"
#include "RenderSystem.h"
#include "Singleton.h"
#include "Transform2d.h"
#include "Camera2d.h"
#include "UnlitMaterial2d.h"
#include "UnlitMaterial.h"

int main()
{
	const uint32_t entityCount = 100;

	ce::Cecsar cecsar{entityCount};
	RenderSystem renderSystem{};
	RenderSystem::Instance::Set(&renderSystem);

	auto& renderer = renderSystem.GetVkRenderer();

	const auto transformSystem = new Transform2d::System(entityCount);
	Transform2d::System::Instance::Set(transformSystem);
	cecsar.AddSet(transformSystem);
	
	const auto meshSystem = new Mesh::System(entityCount);
	Mesh::System::Instance::Set(meshSystem);
	cecsar.AddSet(meshSystem);

	const auto camera2dSystem = new Camera2d::System(entityCount);
	Camera2d::System::Instance::Set(camera2dSystem);
	cecsar.AddSet(camera2dSystem);

	const auto unlitMaterial2dSystem = new UnlitMaterial2d::System(entityCount);
	UnlitMaterial2d::System::Instance::Set(unlitMaterial2dSystem);
	cecsar.AddSet(unlitMaterial2dSystem);

	const auto unlitMaterialSystem = new UnlitMaterial::System(entityCount);
	UnlitMaterial::System::Instance::Set(unlitMaterialSystem);
	cecsar.AddSet(unlitMaterialSystem);

	// Create scene instances.
	const auto camEntity = cecsar.AddEntity();
	camera2dSystem->Insert(camEntity.id);
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
	auto& unlitMaterial = unlitMaterial2dSystem->Insert(quadEntity.index);
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

		camera2dSystem->Update();
		unlitMaterial2dSystem->Update();
		unlitMaterialSystem->Update();

		renderSystem.EndFrame();
	}

	renderer.DeviceWaitIdle();

	renderSystem.DestroyMesh(mesh);
	renderSystem.DestroyMesh(cubeMesh);
	renderSystem.DestroyTexture(texture);

	camera2dSystem->Cleanup();
	delete transformSystem;
	delete camera2dSystem;
	delete meshSystem;
	unlitMaterial2dSystem->Cleanup();
	delete unlitMaterial2dSystem;
	unlitMaterialSystem->Cleanup();
	delete unlitMaterialSystem;
	return 0;
}
