#include "pch.h"
#include "Vertex.h"

VkVertexInputBindingDescription Vertex::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	attributeDescriptions.resize(3);

	auto& position = attributeDescriptions[0];
	position.binding = 0;
	position.location = 0;
	position.format = VK_FORMAT_R32G32B32_SFLOAT;
	position.offset = offsetof(Vertex, position);

	auto& normal = attributeDescriptions[1];
	normal.binding = 0;
	normal.location = 1;
	normal.format = VK_FORMAT_R32G32B32_SFLOAT;
	normal.offset = offsetof(Vertex, normal);

	auto& texCoords = attributeDescriptions[2];
	texCoords.binding = 0;
	texCoords.location = 2;
	texCoords.format = VK_FORMAT_R32G32_SFLOAT;
	texCoords.offset = offsetof(Vertex, textureCoordinates);

	return attributeDescriptions;
}
