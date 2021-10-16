#pragma once
#include "Entity.h"
#include "SparseSet.h"
#include <queue>
#include <vector>

namespace ce
{
	template <size_t S>
	class Cecsar final
	{
	public:
		Cecsar();
		~Cecsar();

		Entity AddEntity();
		void EraseEntity(uint32_t index);

		void AddSet(Set* set);

	private:
		int32_t _globalId = 0;
		SparseSet<Entity, S>* _entities = nullptr;
		std::priority_queue<int32_t, std::vector<int32_t>, std::greater<>> _openPq{};
		
		std::vector<Set*> _sets{};
	};

	template <size_t S>
	Cecsar<S>::Cecsar()
	{
		_entities = new SparseSet<Entity, S>;
	}

	template <size_t Size>
	Cecsar<Size>::~Cecsar()
	{
		delete _entities;
	}

	template <size_t S>
	Entity Cecsar<S>::AddEntity()
	{
		int32_t index = _entities->GetCount();
		if (!_openPq.empty())
		{
			index = _openPq.top();
			_openPq.pop();
		}

		const Entity newEntity
		{
			index,
			_globalId++
		};

		auto& entity = _entities->Insert(index);
		entity = newEntity;
		return entity;
	}

	template <size_t S>
	void Cecsar<S>::EraseEntity(const uint32_t index)
	{
		_entities.Erase(index);
		_openPq.emplace(index);
	}

	template <size_t S>
	void Cecsar<S>::AddSet(Set* set)
	{
		_sets.push_back(set);
	}
}
