#include "pch.h"
#include "Transform3d.h"
#include "glm/ext.hpp"
#include "glm/gtx/euler_angles.hpp"

Transform3d::System::System(const uint32_t size) : SoASet<Transform3d>(size)
{
	AddSubSet<Bake>();
}

void Transform3d::System::Update()
{
	const auto bakes = GetSets()[0].Get<Bake>();

	for (const auto [instance, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);

		auto& bake = bakes[denseId];
		if (bake.manual)
			continue;

		auto& model = bake.model;

		model = glm::mat4{ 1 };
		model = glm::translate(model, instance.position);

		auto& rotation = instance.rotation;
		const auto euler = glm::eulerAngleXYZ(
			glm::radians(rotation.x) / 2,
			glm::radians(rotation.y) / 2,
			glm::radians(rotation.z) / 2);

		model *= euler;
		model = glm::scale(model, instance.scale);
	}
}
