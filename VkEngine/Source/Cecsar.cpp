#include "pch.h"
#include "Cecsar.h"

namespace ce
{
	Cecsar::Cecsar(const uint32_t size) : _entities(size)
	{
		
	}

	Entity Cecsar::AddEntity()
	{
		int32_t index = _entities.GetCount();
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

		auto& entity = _entities.Insert(index);
		entity = newEntity;
		return entity;
	}

	void Cecsar::EraseEntity(const uint32_t index)
	{
		_entities.Erase(index);
		_openPq.emplace(index);
	}

	void Cecsar::AddSet(Set* set)
	{
		_sets.push_back(set);
	}
}