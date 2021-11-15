#pragma once

struct alignas(4) ShadowCaster final
{
	class System final : public ce::SparseSet<ShadowCaster>
	{
	public:
		typedef Singleton<System> Instance;

		explicit System(uint32_t size);
	};
};
