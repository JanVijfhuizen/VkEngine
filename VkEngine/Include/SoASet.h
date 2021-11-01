#pragma once
#include "Cecsar.h"

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

	explicit SoASet(uint32_t size);
	SoASet& operator=(const SoASet& other) = delete;
	~SoASet();

	[[nodiscard]] SubSet operator[](uint32_t setId);
	[[nodiscard]] constexpr std::vector<SubSet>& GetSets();

	constexpr void Insert(uint32_t sparseId);
	void Erase(uint32_t sparseId) override;

	[[nodiscard]] constexpr bool Contains(uint32_t sparseId) const;
	[[nodiscard]] constexpr uint32_t GetCount() const;
	[[nodiscard]] constexpr uint32_t GetSize() const;

	void Swap(uint32_t aDenseId, uint32_t bDenseId);

	[[nodiscard]] constexpr uint32_t GetDenseId(uint32_t sparseId) const;
	[[nodiscard]] constexpr uint32_t GetSparseId(uint32_t denseId) const;

	[[nodiscard]] ce::SparseSet<char>::Iterator begin();
	[[nodiscard]] ce::SparseSet<char>::Iterator end();

	template <typename T>
	void AddSubSet();

private:
	ce::SparseSet<char> _instances{};
	std::vector<SubSet> _subSets{};
};

template <typename T>
constexpr T& SoASet::SubSet::Get(const uint32_t sparseId)
{
	return *reinterpret_cast<T*>(&_data[_unitSize * sparseId]);
}

template <typename T>
void SoASet::AddSubSet()
{
	SubSet set{};
	set._data = reinterpret_cast<char*>(malloc(sizeof(T) * (_instances.GetSize() + 1)));
	set._unitSize = sizeof(T);
	_subSets.push_back(set);
}
