#pragma once

struct Vertex2d final
{
    glm::vec2 position{};
    glm::vec2 textureCoordinates{};

    [[nodiscard]] static VkVertexInputBindingDescription GetBindingDescription();
    [[nodiscard]] static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};
