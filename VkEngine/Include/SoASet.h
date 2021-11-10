#pragma once
#include "Cecsar.h"

namespace ce
{
	template <typename T>
	class SoASet : public SparseSet<T>
	{
	public:
		struct SubSet final
		{
			friend SoASet;

			template <typename U>
			[[nodiscard]] constexpr U* Get();
			template <typename U>
			[[nodiscard]] constexpr U& Get(uint32_t sparseId);

		private:
			char* _data;
			size_t _unitSize;
		};

		explicit SoASet(uint32_t size);
		~SoASet();

		[[nodiscard]] constexpr std::vector<SubSet>& GetSets();

		void Swap(uint32_t aDenseId, uint32_t bDenseId) override;

		template <typename U>
		SubSet AddSubSet();

	private:
		std::vector<SubSet> _subSets{};
	};

	template <typename T>
	template <typename U>
	constexpr U* SoASet<T>::SubSet::Get()
	{
		return reinterpret_cast<U*>(_data);
	}

	template <typename T>
	template <typename U>
	constexpr U& SoASet<T>::SubSet::Get(const uint32_t sparseId)
	{
		return *reinterpret_cast<U*>(&_data[_unitSize * sparseId]);
	}

	template <typename T>
	template <typename U>
	typename SoASet<T>::SubSet SoASet<T>::AddSubSet()
	{
		SubSet set{};
		set._data = reinterpret_cast<char*>(malloc(sizeof(U) * (SparseSet<T>::GetSize() + 1)));
		set._unitSize = sizeof(U);
		_subSets.push_back(set);
		return set;
	}

	template <typename T>
	SoASet<T>::SoASet(const uint32_t size) : SparseSet<T>(size)
	{

	}

	template <typename T>
	SoASet<T>::~SoASet()
	{
		for (const auto& subSet : _subSets)
			free(subSet._data);
	}

	template <typename T>
	constexpr std::vector<typename SoASet<T>::SubSet>& SoASet<T>::GetSets()
	{
		return _subSets;
	}

	template <typename T>
	void SoASet<T>::Swap(const uint32_t aDenseId, const uint32_t bDenseId)
	{
		const uint32_t size = SparseSet<T>::GetSize();
		SparseSet<T>::Swap(aDenseId, bDenseId);

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
}