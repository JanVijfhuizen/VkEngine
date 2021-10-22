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
	attributeDescriptions.resize(2);

	auto& position = attributeDescriptions[0];
	position.binding = 0;
	position.location = 0;
	position.format = VK_FORMAT_R32G32_SFLOAT;
	position.offset = offsetof(Vertex, pos);

	auto& texCoords = attributeDescriptions[1];
	texCoords.binding = 0;
	texCoords.location = 1;
	texCoords.format = VK_FORMAT_R32G32_SFLOAT;
	texCoords.offset = offsetof(Vertex, texCoords);

	return attributeDescriptions;
}
