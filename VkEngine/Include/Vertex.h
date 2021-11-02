#pragma once

struct Vertex
{
    glm::vec2 pos{};
    glm::vec2 texCoords{};

    static VkVertexInputBindingDescription GetBindingDescription();
    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};
