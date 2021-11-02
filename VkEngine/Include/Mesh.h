#pragma once
#include "Vertex.h"

struct Mesh final
{
public:
    // Defaults to a quad.
    struct Info final
    {
        std::vector<Vertex> vertices =
        {
            {{-0.5f, -0.5f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f}}
        };

        std::vector<uint16_t> indices =
        {
            0, 1, 2, 2, 3, 0
        };
    };

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexMemory;
    uint32_t indCount;
};