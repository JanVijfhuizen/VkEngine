#include "pch.h"
#include "Vertex2d.h"

VkVertexInputBindingDescription Vertex2d::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex2d);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Vertex2d::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	attributeDescriptions.resize(2);

	auto& position = attributeDescriptions[0];
	position.binding = 0;
	position.location = 0;
	position.format = VK_FORMAT_R32G32_SFLOAT;
	position.offset = offsetof(Vertex2d, position);

	auto& texCoords = attributeDescriptions[1];
	texCoords.binding = 0;
	texCoords.location = 1;
	texCoords.format = VK_FORMAT_R32G32_SFLOAT;
	texCoords.offset = offsetof(Vertex2d, textureCoordinates);

	return attributeDescriptions;
}
