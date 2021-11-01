#pragma once
#include <cstdint>
#include <utility>
#include "Set.h"

namespace ce
{
	template <typename T>
	class SparseSet : public Set
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
			explicit Iterator(SparseSet<T>& set, uint32_t index);

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
			SparseSet<T>& _set;
		};

		SparseSet();
		explicit SparseSet(uint32_t size);
		SparseSet<T>& operator=(const SparseSet<T>& other) = delete;
		~SparseSet();

		[[nodiscard]] constexpr T& operator[](uint32_t sparseId);

		virtual T& Insert(uint32_t sparseId);
		void Erase(uint32_t sparseId) override;

		[[nodiscard]] constexpr bool Contains(uint32_t sparseId) const;
		[[nodiscard]] constexpr uint32_t GetCount() const;
		[[nodiscard]] constexpr uint32_t GetSize() const;

		virtual void Swap(uint32_t aDenseId, uint32_t bDenseId);

		[[nodiscard]] constexpr uint32_t GetDenseId(uint32_t sparseId) const;
		[[nodiscard]] constexpr uint32_t GetSparseId(uint32_t denseId) const;

		[[nodiscard]] constexpr Iterator begin();
		[[nodiscard]] constexpr Iterator end();

	private:
		char* _data = nullptr;
		T* _values;
		uint32_t* _dense;
		uint32_t* _sparse;

		uint32_t _count = 0;
		uint32_t _size;
	};

	template <typename T>
	SparseSet<T>::Iterator::Iterator(SparseSet<T>& set, const uint32_t index) : _index(index), _set(set)
	{
	}

	template <typename T>
	typename SparseSet<T>::Value SparseSet<T>::Iterator::operator*() const
	{
		return { _set._values[_index], _set._dense[_index] };
	}

	template <typename T>
	typename SparseSet<T>::Value SparseSet<T>::Iterator::operator->() const
	{
		return { _set._values[_index], _set._dense[_index] };
	}

	template <typename T>
	const typename SparseSet<T>::Iterator& SparseSet<T>::Iterator::operator++()
	{
		++_index;
		return *this;
	}

	template <typename T>
	typename SparseSet<T>::Iterator SparseSet<T>::Iterator::operator++(int)
	{
		Iterator temp{ *this };
		++_index;
		return temp;
	}

	template <typename T>
	SparseSet<T>::SparseSet() = default;

	template <typename T>
	SparseSet<T>::SparseSet(const uint32_t size) : _size(size)
	{
		const size_t valSize = sizeof(T) * size;
		const size_t iSize = sizeof(uint32_t) * size;
		const size_t memSize = valSize + iSize * 2;

		_data = reinterpret_cast<char*>(malloc(memSize));
		_values = reinterpret_cast<T*>(_data);
		_dense = reinterpret_cast<uint32_t*>(_data + iSize);
		_sparse = _dense + iSize;

		for (uint32_t i = 0; i < size; ++i)
			_sparse[i] = -1;
	}

	template <typename T>
	SparseSet<T>::~SparseSet()
	{
		free(_data);
	}

	template <typename T>
	constexpr T& SparseSet<T>::operator[](const uint32_t sparseId)
	{
		return _values[_sparse[sparseId]];
	}

	template <typename T>
	T& SparseSet<T>::Insert(const uint32_t sparseId)
	{
		if(!Contains(sparseId))
		{
			_sparse[sparseId] = _count;
			_values[_count] = {};
			_dense[_count++] = sparseId;
		}

		return _values[sparseId];
	}

	template <typename T>
	void SparseSet<T>::Erase(const uint32_t sparseId)
	{
		const int32_t denseId = _sparse[sparseId];
		Swap(denseId, --_count);

		_sparse[sparseId] = -1;
		_values[_count] = T();
	}

	template <typename T>
	constexpr bool SparseSet<T>::Contains(const uint32_t sparseId) const
	{
		return _sparse[sparseId] != -1;
	}

	template <typename T>
	constexpr uint32_t SparseSet<T>::GetCount() const
	{
		return _count;
	}

	template <typename T>
	constexpr uint32_t SparseSet<T>::GetSize() const
	{
		return _size;
	}

	template <typename T>
	void SparseSet<T>::Swap(const uint32_t aDenseId, const uint32_t bDenseId)
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

	template <typename T>
	constexpr uint32_t SparseSet<T>::GetDenseId(const uint32_t sparseId) const
	{
		return _sparse[sparseId];
	}

	template <typename T>
	constexpr uint32_t SparseSet<T>::GetSparseId(const uint32_t denseId) const
	{
		return _dense[denseId];
	}

	template <typename T>
	constexpr typename SparseSet<T>::Iterator SparseSet<T>::begin()
	{
		return Iterator{ *this, 0 };
	}

	template <typename T>
	constexpr typename SparseSet<T>::Iterator SparseSet<T>::end()
	{
		return Iterator{ *this, _count };
	}
}
