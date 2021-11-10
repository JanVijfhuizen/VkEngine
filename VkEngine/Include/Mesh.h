#pragma once
#include "Vertex2d.h"

struct Mesh final
{
    // Defaults to a quad.
    struct Info final
    {
        std::vector<Vertex2d> vertices =
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

    class System final : public ce::SparseSet<Mesh>
    {
    public:
        typedef Singleton<System> Instance;

        explicit System(uint32_t size);
    };
};

inline Mesh::System::System(const uint32_t size) : SparseSet<Mesh>(size)
{

}
