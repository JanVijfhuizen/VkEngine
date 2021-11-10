#pragma once
#include "Vertex2d.h"
#include "Vertex.h"

struct Mesh final
{
    // Defaults to a quad.
    struct Info2d final
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

        static void LoadMesh(const std::string& path, std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices);
    };
};
