#include "pch.h"
#include "SoASet.h"

SoASet::SoASet(const uint32_t size) : _instances(size)
{
	
}

SoASet::~SoASet()
{
	for (const auto& subSet : _subSets)
		free(subSet._data);
}

SoASet::SubSet SoASet::operator[](const uint32_t setId)
{
	return _subSets[setId];
}

constexpr std::vector<SoASet::SubSet>& SoASet::GetSets()
{
	return _subSets;
}

constexpr void SoASet::Insert(const uint32_t sparseId)
{
	_instances.Insert(sparseId);
}

void SoASet::Erase(const uint32_t sparseId)
{
	_instances.Erase(sparseId);
}

constexpr bool SoASet::Contains(const uint32_t sparseId) const
{
	return _instances.Contains(sparseId);
}

constexpr uint32_t SoASet::GetCount() const
{
	return _instances.GetCount();
}

constexpr uint32_t SoASet::GetSize() const
{
	return _instances.GetSize();
}

void SoASet::Swap(const uint32_t aDenseId, const uint32_t bDenseId)
{
	const uint32_t size = GetSize();
	_instances.Swap(aDenseId, bDenseId);

	for (const auto& subSet : _subSets)
	{
		const auto data = subSet._data;
		const auto& unitSize = subSet._unitSize;

		char* aDense = &data[unitSize * aDenseId];
		char* bDense = &data[unitSize * bDenseId];
		char* tempStorage = &data[unitSize * size];

		memcpy(tempStorage, aDense, unitSize);
		memcpy(aDense, bDense, unitSize);
		memcpy(bDense, tempStorage, unitSize);
	}
}

constexpr uint32_t SoASet::GetDenseId(const uint32_t sparseId) const
{
	return _instances.GetDenseId(sparseId);
}

constexpr uint32_t SoASet::GetSparseId(const uint32_t denseId) const
{
	return _instances.GetSparseId(denseId);
}

ce::SparseSet<char>::Iterator SoASet::begin()
{
	return _instances.begin();
}

ce::SparseSet<char>::Iterator SoASet::end()
{
	return _instances.end();
}