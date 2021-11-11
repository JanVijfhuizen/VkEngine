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
#include "Camera3d.h"

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

	const auto camera2dSystem = new Camera2d::System(entityCount);
	Camera2d::System::Instance::Set(camera2dSystem);
	cecsar.AddSet(camera2dSystem);

	const auto camera3dSystem = new Camera3d::System(entityCount);
	Camera3d::System::Instance::Set(camera3dSystem);
	cecsar.AddSet(camera3dSystem);

	const auto unlitMaterial2dSystem = new UnlitMaterial2d::System(entityCount);
	UnlitMaterial2d::System::Instance::Set(unlitMaterial2dSystem);
	cecsar.AddSet(unlitMaterial2dSystem);

	const auto unlitMaterial3dSystem = new UnlitMaterial3d::System(entityCount);
	UnlitMaterial3d::System::Instance::Set(unlitMaterial3dSystem);
	cecsar.AddSet(unlitMaterial3dSystem);

	// Create scene instances.
	auto texture = renderSystem.CreateTexture("Example.jpg");

	// Add quad entity + camera.
	const auto cam2dEntity = cecsar.AddEntity();
	camera2dSystem->Insert(cam2dEntity.id);
	transform2dSystem->Insert(cam2dEntity.id);

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
	const auto cam3dEntity = cecsar.AddEntity();
	camera3dSystem->Insert(cam3dEntity.id);
	auto& cam3dTransform = transform3dSystem->Insert(cam3dEntity.id);

	std::vector<Vertex3d> cubeVerts{};
	std::vector<uint16_t> cubeInds{};
	Mesh::System::Load("Cube.obj", cubeVerts, cubeInds);

	const auto cubeEntity = cecsar.AddEntity();
	auto& cubeTransform = transform3dSystem->Insert(cubeEntity.index);
	cubeTransform.position = { 1, .5f, -1 };
	cubeTransform.scale = glm::vec3{ .5f };
	auto& cubeMesh = meshSystem->Insert(cubeEntity.index);
	cubeMesh = renderSystem.CreateMesh<Vertex3d, uint16_t>(cubeVerts, cubeInds);
	auto& unlitMaterial3d = unlitMaterial3dSystem->Insert(cubeEntity.index);
	unlitMaterial3d.diffuseTexture = &texture;
	
	const auto cube2Entity = cecsar.AddEntity();
	auto& cube2Transform = transform3dSystem->Insert(cube2Entity.index);
	cube2Transform.position = { 1, .5f, -1 };
	auto& cube2Mesh = meshSystem->Insert(cube2Entity.index);
	cube2Mesh = cubeMesh;
	auto& unlitMaterial3d2 = unlitMaterial3dSystem->Insert(cube2Entity.index);
	unlitMaterial3d2.diffuseTexture = &texture;

	const auto cube3Entity = cecsar.AddEntity();
	auto& cube3Transform = transform3dSystem->Insert(cube3Entity.index);
	cube3Transform.position = { 0, 0, 0 };
	cube3Transform.rotation = { 45, 38, 12 };
	auto& cube3Mesh = meshSystem->Insert(cube3Entity.index);
	cube3Mesh = cubeMesh;
	auto& unlitMaterial3d3 = unlitMaterial3dSystem->Insert(cube3Entity.index);
	unlitMaterial3d3.diffuseTexture = &texture;
	
	while(true)
	{
		bool quit;
		renderSystem.BeginFrame(&quit);
		if (quit)
			break;

		static float f = 0;
		f += .001f;
		cam3dTransform.position = { std::sin(f) * 30, std::sin(f * 4) * 10, std::cos(f) * 30};

		transform3dSystem->Update();
		camera2dSystem->Update();
		camera3dSystem->Update();

		unlitMaterial2dSystem->Update();
		unlitMaterial3dSystem->Update();

		renderSystem.EndFrame();
	}

	renderer.DeviceWaitIdle();

	renderSystem.DestroyMesh(quadMesh);
	renderSystem.DestroyMesh(cubeMesh);
	renderSystem.DestroyTexture(texture);

	camera2dSystem->Cleanup();
	delete camera2dSystem;
	camera3dSystem->Cleanup();
	delete camera3dSystem;

	delete transform2dSystem;
	delete transform3dSystem;
	delete meshSystem;
	
	unlitMaterial2dSystem->Cleanup();
	delete unlitMaterial2dSystem;
	unlitMaterial3dSystem->Cleanup();
	delete unlitMaterial3dSystem;
	return 0;
}
