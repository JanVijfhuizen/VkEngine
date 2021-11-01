#pragma once
#include "Cecsar.h"

template <size_t S>
class SoASet final : public ce::Set
{
public:
	struct SubSet final
	{
		friend SoASet;

		template <typename T>
		[[nodiscard]] constexpr T& Get(uint32_t sparseId);

	private:
		char* _data;
		size_t _unitSize;
	};

	~SoASet();

	[[nodiscard]] constexpr SubSet operator[](uint32_t setId);
	[[nodiscard]] constexpr std::vector<SubSet>& GetSets();

	constexpr void Insert(uint32_t sparseId);
	constexpr void Erase(uint32_t sparseId) override;

	[[nodiscard]] constexpr bool Contains(uint32_t sparseId) const;
	[[nodiscard]] constexpr uint32_t GetCount() const;

	constexpr void Swap(uint32_t aDenseId, uint32_t bDenseId);

	[[nodiscard]] constexpr uint32_t GetDenseId(uint32_t sparseId);
	[[nodiscard]] constexpr uint32_t GetSparseId(uint32_t denseId);

	[[nodiscard]] constexpr typename ce::SparseSet<uint32_t, S>::Iterator begin();
	[[nodiscard]] constexpr typename ce::SparseSet<uint32_t, S>::Iterator end();

	template <typename T>
	void AddSubSet();

private:
	ce::SparseSet<char, S> _instances{};
	std::vector<SubSet> _subSets{};
};

template <size_t S>
constexpr typename SoASet<S>::SubSet SoASet<S>::operator[](const uint32_t setId)
{
	return _subSets[setId];
}

template <size_t S>
constexpr std::vector<typename SoASet<S>::SubSet>& SoASet<S>::GetSets()
{
	return _subSets;
}

template <size_t S>
constexpr void SoASet<S>::Insert(const uint32_t sparseId)
{
	_instances.Insert(sparseId);
}

template <size_t S>
constexpr void SoASet<S>::Erase(const uint32_t sparseId)
{
	_instances.Erase(sparseId);
}

template <size_t S>
constexpr bool SoASet<S>::Contains(const uint32_t sparseId) const
{
	return _instances.Contains(sparseId);
}

template <size_t S>
constexpr uint32_t SoASet<S>::GetCount() const
{
	return _instances.GetCount();
}

template <size_t S>
constexpr void SoASet<S>::Swap(const uint32_t aDenseId, const uint32_t bDenseId)
{
	_instances.Swap(aDenseId, bDenseId);

	for (const auto& subSet : _subSets)
	{
		const auto data = subSet.data;
		const auto& unitSize = subSet.unitSize;

		void* aDense = data[unitSize * aDenseId];
		void* bDense = data[unitSize * bDenseId];
		void* tempStorage = data[unitSize * S];

		memcpy(tempStorage, aDense, unitSize);
		memcpy(aDense, bDense, unitSize);
		memcpy(bDense, tempStorage, unitSize);
	}
}

template <size_t S>
constexpr uint32_t SoASet<S>::GetDenseId(const uint32_t sparseId)
{
	return _instances.GetDenseId(sparseId);
}

template <size_t S>
constexpr uint32_t SoASet<S>::GetSparseId(const uint32_t denseId)
{
	return _instances.GetSparseId(denseId);
}

template <size_t S>
constexpr typename ce::SparseSet<uint32_t, S>::Iterator SoASet<S>::begin()
{
	return _instances.begin();
}

template <size_t S>
constexpr typename ce::SparseSet<uint32_t, S>::Iterator SoASet<S>::end()
{
	return _instances.end();
}

template <size_t S>
template <typename T>
constexpr T& SoASet<S>::SubSet::Get(const uint32_t sparseId)
{
	return *reinterpret_cast<T*>(&_data[_unitSize * sparseId]);
}

template <size_t S>
SoASet<S>::~SoASet()
{
	for (const auto& subSet : _subSets)
		free(subSet._data);
}

template <size_t S>
template <typename T>
void SoASet<S>::AddSubSet()
{
	SubSet set{};
	set._data = reinterpret_cast<char*>(malloc(sizeof(T) * (S + 1)));
	set._unitSize = sizeof(T);
	_subSets.push_back(set);
}
