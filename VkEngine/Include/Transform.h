#pragma once

struct Transform final
{
	glm::vec2 transPos{};
	glm::vec2 transScale{ 1 };
	float transRot = 0;
};