#pragma once
#include "Entity.h"
#include "SparseSet.h"
#include <queue>
#include <vector>

namespace ce
{
	class Cecsar
	{
	public:
		explicit Cecsar(uint32_t size);

		Entity AddEntity();
		void EraseEntity(uint32_t index);

		void AddSet(Set* set);

	private:
		int32_t _globalId = 0;
		SparseSet<Entity> _entities;
		std::priority_queue<int32_t, std::vector<int32_t>, std::greater<>> _openPq{};
		std::vector<Set*> _sets{};
	};
}
