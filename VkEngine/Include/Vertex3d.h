#pragma once

struct Vertex3d final
{
	glm::vec3 position{};
	glm::vec3 normal{0, 0, 1};
	glm::vec2 textureCoordinates{};

	[[nodiscard]] static VkVertexInputBindingDescription GetBindingDescription();
	[[nodiscard]] static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};
