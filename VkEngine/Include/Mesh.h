#pragma once
#include "Vertex2d.h"
#include "Vertex3d.h"

struct Mesh final
{
    // Defaults to a quad.
    struct Quad final
    {
        std::vector<Vertex2d> vertices =
        {
            {{-1, -1}, {1, 0}},
            {{1, -1}, {0, 0}},
            {{1, 1}, {0, 1}},
            {{-1, 1}, {1, 1}}
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

    class System final : public ce::SparseSet<Mesh>
    {
    public:
        typedef Singleton<System> Instance;

        explicit System(uint32_t size);

        static void Load(const std::string& fileName, std::vector<Vertex3d>& outVertices, std::vector<int8_t>& outIndices);
    };
};
