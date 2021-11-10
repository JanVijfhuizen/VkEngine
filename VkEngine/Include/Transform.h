#pragma once

struct Transform final
{
	glm::vec2 position{};
	glm::vec2 scale{ 1 };
	float rotation = 0;

	class System final : public ce::SparseSet<Transform>
	{
	public:
		typedef Singleton<System> Instance;

		explicit System(uint32_t size);
	};
};
