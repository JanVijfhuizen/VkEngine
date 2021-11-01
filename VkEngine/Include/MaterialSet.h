#pragma once
#include "SoASet.h"
#include "RenderSystem.h"
#include "Singleton.h"

template <typename Material, typename Frame>
class MaterialSet : public ce::SoASet<Material>
{
public:
	explicit MaterialSet(uint32_t size);

	Material& Insert(uint32_t sparseId) override;
	void Erase(uint32_t sparseId) override;

	virtual void ConstructInstance(uint32_t denseId);
	virtual void CleanupInstance(uint32_t denseId);

	void Update();

private:
	std::vector<uint32_t> _erasableIds{};
};

template <typename Material, typename Frame>
MaterialSet<Material, Frame>::MaterialSet(const uint32_t size) : ce::SoASet<Material>(size)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& swapChain = renderSystem.GetSwapChain();

	ce::SoASet<Material>::template AddSubSet<int8_t>();
	const uint32_t imageCount = swapChain.GetImageCount();
	for (uint32_t i = 0; i < imageCount; ++i)
		ce::SoASet<Material>::template AddSubSet<Frame>();
}

template <typename Material, typename Frame>
Material& MaterialSet<Material, Frame>::Insert(const uint32_t sparseId)
{
	auto& material = ce::SoASet<Material>::Insert(sparseId);
	auto& deleteQueue = ce::SoASet<Material>::GetSets()[0];
	const uint32_t denseId = ce::SoASet<Material>::GetDenseId(sparseId);

	deleteQueue.template Get<int8_t>(denseId) = -1;
	ConstructInstance(denseId);
	return material;
}

template <typename Material, typename Frame>
void MaterialSet<Material, Frame>::Erase(const uint32_t sparseId)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& swapChain = renderSystem.GetSwapChain();

	auto& deleteQueue = ce::SoASet<Material>::GetSets()[0];
	deleteQueue.template Get<int8_t>(ce::SoASet<Material>::GetDenseId(sparseId)) = swapChain.GetImageCount();
}

template <typename Material, typename Frame>
void MaterialSet<Material, Frame>::ConstructInstance(const uint32_t denseId)
{
}

template <typename Material, typename Frame>
void MaterialSet<Material, Frame>::CleanupInstance(const uint32_t denseId)
{
}

template <typename Material, typename Frame>
void MaterialSet<Material, Frame>::Update()
{
	_erasableIds.clear();
	auto& deleteQueue = ce::SoASet<Material>::GetSets()[0];

	for (const auto [material, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);

		auto& countdown = deleteQueue.Get<int8_t>(denseId);
		if (countdown == -1)
			continue;

		if (countdown-- > 0)
			continue;

		CleanupInstance(denseId);
		_erasableIds.push_back(sparseId);
	}

	for (const auto& id : _erasableIds)
		ce::SoASet<Material>::Erase(id);
}

