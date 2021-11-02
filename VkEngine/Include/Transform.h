#pragma once

struct Transform final
{
	glm::vec2 position{};
	glm::vec2 scale{ 1 };
	float rotation = 0;
};