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
#include "UnlitMaterial3d.h"
#include "Transform3d.h"

int main()
{
	const uint32_t entityCount = 100;

	ce::Cecsar cecsar{entityCount};
	RenderSystem renderSystem{};
	RenderSystem::Instance::Set(&renderSystem);

	auto& renderer = renderSystem.GetVkRenderer();

	const auto transform2dSystem = new Transform2d::System(entityCount);
	Transform2d::System::Instance::Set(transform2dSystem);
	cecsar.AddSet(transform2dSystem);

	const auto transform3dSystem = new Transform3d::System(entityCount);
	Transform3d::System::Instance::Set(transform3dSystem);
	cecsar.AddSet(transform3dSystem);
	
	const auto meshSystem = new Mesh::System(entityCount);
	Mesh::System::Instance::Set(meshSystem);
	cecsar.AddSet(meshSystem);

	const auto camera2dSystem = new Camera2dSystem(entityCount);
	Camera2dSystem::Instance::Set(camera2dSystem);
	cecsar.AddSet(camera2dSystem);

	const auto unlitMaterial2dSystem = new UnlitMaterial2d::System(entityCount);
	UnlitMaterial2d::System::Instance::Set(unlitMaterial2dSystem);
	cecsar.AddSet(unlitMaterial2dSystem);

	const auto unlitMaterial3dSystem = new UnlitMaterial3d::System(entityCount);
	UnlitMaterial3d::System::Instance::Set(unlitMaterial3dSystem);
	cecsar.AddSet(unlitMaterial3dSystem);

	// Create scene instances.
	const auto cam2dEntity = cecsar.AddEntity();
	camera2dSystem->Insert(cam2dEntity.id);
	transform2dSystem->Insert(cam2dEntity.id);

	auto texture = renderSystem.CreateTexture("Example.jpg");

	// Add quad entity.
	Mesh::Quad quadInfo{};
	for (auto& vertex : quadInfo.vertices)
		vertex.position /= 2;

	const auto quadEntity = cecsar.AddEntity();
	transform2dSystem->Insert(quadEntity.index);
	auto& quadMesh = meshSystem->Insert(quadEntity.index);
	quadMesh = renderSystem.CreateMesh(quadInfo.vertices, quadInfo.indices);
	auto& unlitMaterial2d = unlitMaterial2dSystem->Insert(quadEntity.index);
	unlitMaterial2d.diffuseTexture = &texture;

	// Add cube entity.
	std::vector<Vertex3d> cubeVerts{};
	std::vector<int8_t> cubeInds{};
	Mesh::System::Load("Cube.obj", cubeVerts, cubeInds);

	const auto cubeEntity = cecsar.AddEntity();
	transform2dSystem->Insert(cubeEntity.index);
	auto& cubeMesh = meshSystem->Insert(cubeEntity.index);
	cubeMesh = renderSystem.CreateMesh<Vertex3d, int8_t>(cubeVerts, cubeInds);

	while(true)
	{
		bool quit;
		renderSystem.BeginFrame(&quit);
		if (quit)
			break;

		transform3dSystem->Update();
		camera2dSystem->Update();

		unlitMaterial2dSystem->Update();
		unlitMaterial3dSystem->Update();

		renderSystem.EndFrame();
	}

	renderer.DeviceWaitIdle();

	renderSystem.DestroyMesh(quadMesh);
	renderSystem.DestroyMesh(cubeMesh);
	renderSystem.DestroyTexture(texture);

	camera2dSystem->Cleanup();
	delete transform2dSystem;
	delete transform3dSystem;
	delete camera2dSystem;
	delete meshSystem;
	unlitMaterial2dSystem->Cleanup();
	delete unlitMaterial2dSystem;
	unlitMaterial3dSystem->Cleanup();
	delete unlitMaterial3dSystem;
	return 0;
}
