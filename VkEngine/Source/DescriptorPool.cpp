#include "pch.h"
#include "DescriptorPool.h"
#include "RenderSystem.h"
#include "Singleton.h"

void DescriptorPool::Construct(const uint32_t size, const VkDescriptorSetLayout layout, VkDescriptorType* types, const uint32_t typeCount)
{
	Pool<VkDescriptorSet>::Construct(size);

	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	_layout = layout;
	_descriptorPool = renderer.CreateDescriptorPool(types, typeCount, size);
	_remainingSetsInPool = size;
}

void DescriptorPool::Cleanup()
{
	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	renderer.DestroyLayout(_layout);
	renderer.DestroyDescriptorPool(_descriptorPool);

	Pool<VkDescriptorSet>::Cleanup();
}

VkDescriptorSet DescriptorPool::Get()
{
	if (_remainingSetsInPool == 0)
		return Pool<VkDescriptorSet>::Get();
	_remainingSetsInPool--;

	auto& renderSystem = Singleton<RenderSystem>::Get();
	auto& renderer = renderSystem.GetVkRenderer();

	VkDescriptorSet set;
	renderer.CreateDescriptorSets(_descriptorPool, _layout, &set, 1);
	return set;
}
