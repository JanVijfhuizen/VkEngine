﻿#pragma once
#include "SoASet.h"
#include "RenderSystem.h"
#include "Singleton.h"

template <typename Material, typename Frame>
class ShaderSet : public ce::SoASet<Material>
{
public:
	explicit ShaderSet(uint32_t size);

	Material& Insert(uint32_t sparseId) override;
	void Erase(uint32_t sparseId) override;

	virtual void ConstructInstance(Material& material, uint32_t denseId);
	virtual void CleanupInstance(Material& material, uint32_t denseId);
	virtual void ConstructInstanceFrame(Frame& frame, Material& material, uint32_t denseId);
	virtual void CleanupInstanceFrame(Frame& frame, Material& material, uint32_t denseId);

	virtual void Update();

protected:
	typename ce::SoASet<Material>::SubSet GetCurrentFrameSet();

private:
	std::vector<uint32_t> _erasableIds{};
};

template <typename Material, typename Frame>
ShaderSet<Material, Frame>::ShaderSet(const uint32_t size) : ce::SoASet<Material>(size)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& swapChain = renderSystem.GetSwapChain();

	ce::SoASet<Material>::template AddSubSet<int8_t>();

	const uint32_t imageCount = swapChain.GetImageCount();
	for (uint32_t i = 0; i < imageCount; ++i)
		ce::SoASet<Material>::template AddSubSet<Frame>();
}

template <typename Material, typename Frame>
Material& ShaderSet<Material, Frame>::Insert(const uint32_t sparseId)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& swapChain = renderSystem.GetSwapChain();

	auto& material = ce::SoASet<Material>::Insert(sparseId);
	auto& deleteQueue = ce::SoASet<Material>::GetSets()[0];
	const uint32_t denseId = ce::SoASet<Material>::GetDenseId(sparseId);

	deleteQueue.template Get<int8_t>(denseId) = -1;
	ConstructInstance(denseId);

	const uint32_t imageCount = swapChain.GetImageCount();
	auto& sets = ce::SoASet<Material>::GetSets();

	for (uint32_t i = 1; i < imageCount + 1; ++i)
		ConstructInstanceFrame(sets[i], material, denseId);

	return material;
}

template <typename Material, typename Frame>
void ShaderSet<Material, Frame>::Erase(const uint32_t sparseId)
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& swapChain = renderSystem.GetSwapChain();

	auto& deleteQueue = ce::SoASet<Material>::GetSets()[0];
	deleteQueue.template Get<int8_t>(ce::SoASet<Material>::GetDenseId(sparseId)) = swapChain.GetImageCount();
}

template <typename Material, typename Frame>
void ShaderSet<Material, Frame>::ConstructInstance(Material& material, const uint32_t denseId)
{
}

template <typename Material, typename Frame>
void ShaderSet<Material, Frame>::CleanupInstance(Material& material, const uint32_t denseId)
{
}

template <typename Material, typename Frame>
void ShaderSet<Material, Frame>::ConstructInstanceFrame(Frame& frame, Material& material, const uint32_t denseId)
{
}

template <typename Material, typename Frame>
void ShaderSet<Material, Frame>::CleanupInstanceFrame(Frame& frame, Material& material, const uint32_t denseId)
{
}

template <typename Material, typename Frame>
void ShaderSet<Material, Frame>::Update()
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& swapChain = renderSystem.GetSwapChain();

	const uint32_t imageCount = swapChain.GetImageCount();
	auto& sets = ce::SoASet<Material>::GetSets();
	auto& deleteQueue = ce::SoASet<Material>::GetSets()[0];

	_erasableIds.clear();

	for (const auto [material, sparseId] : *this)
	{
		const uint32_t denseId = GetDenseId(sparseId);

		auto& countdown = deleteQueue.Get<int8_t>(denseId);
		if (countdown == -1)
			continue;

		if (countdown-- > 0)
			continue;

		CleanupInstance(denseId);

		for (uint32_t i = 1; i < imageCount + 1; ++i)
			CleanupInstanceFrame(sets[i], material, denseId);

		_erasableIds.push_back(sparseId);
	}

	for (const auto& id : _erasableIds)
		ce::SoASet<Material>::Erase(id);
}

template <typename Material, typename Frame>
typename ce::SoASet<Material>::SubSet ShaderSet<Material, Frame>::GetCurrentFrameSet()
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& swapChain = renderSystem.GetSwapChain();
	auto& sets = ce::SoASet<Material>::GetSets();

	return sets[swapChain.GetCurrentImageIndex() + 1];
}

