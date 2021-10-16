#pragma once
#include <cstdint>
#include <utility>
#include "Set.h"

namespace ce
{
	template <typename T, size_t S>
	class SparseSet final : public Set
	{
	public:
		struct Value final
		{
			T& value;
			const int32_t index;
		};

		class Iterator final
		{
		public:
			explicit Iterator(SparseSet<T, S>& set, uint32_t index);

			Value operator*() const;
			Value operator->() const;

			const Iterator& operator++();
			Iterator operator++(int);

			friend auto operator==(const Iterator& a, const Iterator& b) -> bool
			{
				return a._index == b._index;
			};

			friend bool operator!= (const Iterator& a, const Iterator& b)
			{
				return !(a == b);
			};

		private:
			uint32_t _index = 0;
			SparseSet<T, S>& _set;
		};

		[[nodiscard]] constexpr T& operator[](uint32_t sparseId);

		[[nodiscard]] constexpr T& Insert(uint32_t sparseId);
		constexpr void Erase(uint32_t sparseId) override;

		[[nodiscard]] constexpr bool Contains(uint32_t sparseId) const;
		[[nodiscard]] constexpr uint32_t GetCount() const;

		constexpr void Swap(uint32_t aDenseId, uint32_t bDenseId);

		[[nodiscard]] constexpr Iterator begin();
		[[nodiscard]] constexpr Iterator end();

	private:
		T _values[S]{};
		int32_t _dense[S]{ -1 };
		int32_t _sparse[S]{ -1 };

		uint32_t _count = 0;
	};

	template <typename T, size_t S>
	SparseSet<T, S>::Iterator::Iterator(SparseSet<T, S>& set, const uint32_t index) : _index(index), _set(set)
	{
	}

	template <typename T, size_t S>
	typename SparseSet<T, S>::Value SparseSet<T, S>::Iterator::operator*() const
	{
		return { _set._values[_index], _set._dense[_index] };
	}

	template <typename T, size_t S>
	typename SparseSet<T, S>::Value SparseSet<T, S>::Iterator::operator->() const
	{
		return { _set._values[_index], _set._dense[_index] };
	}

	template <typename T, size_t S>
	const typename SparseSet<T, S>::Iterator& SparseSet<T, S>::Iterator::operator++()
	{
		++_index;
		return *this;
	}

	template <typename T, size_t S>
	typename SparseSet<T, S>::Iterator SparseSet<T, S>::Iterator::operator++(int)
	{
		Iterator temp{ *this };
		++_index;
		return temp;
	}

	template <typename T, size_t Size>
	constexpr T& SparseSet<T, Size>::operator[](const uint32_t sparseId)
	{
		return _values[_sparse[sparseId]];
	}

	template <typename T, size_t Size>
	constexpr T& SparseSet<T, Size>::Insert(const uint32_t sparseId)
	{
		if(!Contains(sparseId))
		{
			_sparse[sparseId] = _count;
			_values[_count] = {};
			_dense[_count++] = sparseId;
		}

		return _values[sparseId];
	}

	template <typename T, size_t Size>
	constexpr void SparseSet<T, Size>::Erase(const uint32_t sparseId)
	{
		const int32_t denseId = _sparse[sparseId];
		Swap(denseId, --_count);

		_sparse[sparseId] = -1;
		_values[_count] = T();
	}

	template <typename T, size_t Size>
	constexpr bool SparseSet<T, Size>::Contains(const uint32_t sparseId) const
	{
		return _sparse[sparseId] != -1;
	}

	template <typename T, size_t Size>
	constexpr uint32_t SparseSet<T, Size>::GetCount() const
	{
		return _count;
	}

	template <typename T, size_t Size>
	constexpr void SparseSet<T, Size>::Swap(const uint32_t aDenseId, const uint32_t bDenseId)
	{
		const int32_t aSparse = _dense[aDenseId];
		const int32_t bSparse = _dense[aDenseId] = _dense[bDenseId];
		_dense[bDenseId] = aSparse;

		T aValue = std::move(_values[aDenseId]);
		_values[aDenseId] = std::move(_values[bDenseId]);
		_values[bDenseId] = std::move(aValue);

		_sparse[aSparse] = bDenseId;
		_sparse[bSparse] = aDenseId;
	}

	template <typename T, size_t S>
	constexpr typename SparseSet<T, S>::Iterator SparseSet<T, S>::begin()
	{
		return Iterator{ *this, 0 };
	}

	template <typename T, size_t S>
	constexpr typename SparseSet<T, S>::Iterator SparseSet<T, S>::end()
	{
		return Iterator{ *this, _count };
	}
}
