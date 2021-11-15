#include "pch.h"
#include "Vertex3d.h"

VkVertexInputBindingDescription Vertex3d::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex3d);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Vertex3d::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	attributeDescriptions.resize(3);

	auto& position = attributeDescriptions[0];
	position.binding = 0;
	position.location = 0;
	position.format = VK_FORMAT_R32G32B32_SFLOAT;
	position.offset = offsetof(Vertex3d, position);

	auto& normal = attributeDescriptions[1];
	normal.binding = 0;
	normal.location = 1;
	normal.format = VK_FORMAT_R32G32B32_SFLOAT;
	normal.offset = offsetof(Vertex3d, normal);

	auto& texCoords = attributeDescriptions[2];
	texCoords.binding = 0;
	texCoords.location = 2;
	texCoords.format = VK_FORMAT_R32G32_SFLOAT;
	texCoords.offset = offsetof(Vertex3d, textureCoordinates);

	return attributeDescriptions;
}
