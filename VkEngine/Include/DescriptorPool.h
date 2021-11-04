#pragma once
#include "Pool.h"

class DescriptorPool final : private Pool<VkDescriptorSet>
{
public:
	using Pool<VkDescriptorSet>::Get;
	using Pool<VkDescriptorSet>::Add;

	void Construct(uint32_t size, VkDescriptorSetLayout layout, VkDescriptorType* types, uint32_t typeCount);
	void Cleanup() override;

	[[nodiscard]] VkDescriptorSet Get() override;

private:
	VkDescriptorSetLayout _layout;
	VkDescriptorPool _descriptorPool;
	uint32_t _remainingSetsInPool;
};
