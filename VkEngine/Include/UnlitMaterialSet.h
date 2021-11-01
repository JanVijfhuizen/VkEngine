#pragma once
#include "MaterialSet.h"

struct UnlitMaterial final
{
	struct Frame final
	{
		VkDescriptorSet camSet;
		VkBuffer camBuffer;
		VkDeviceMemory camMemory;

		VkDescriptorSet matSet;
		VkSampler matDiffuseSampler;
	};

	class System final : public MaterialSet<UnlitMaterial, Frame>
	{
	public:
		explicit System(uint32_t size);
		void Cleanup();

		void ConstructInstance(uint32_t denseId) override;
		void CleanupInstance(uint32_t denseId) override;

	private:
		vi::Pipeline _pipeline;
		VkDescriptorSetLayout _camLayout;
		VkDescriptorSetLayout _materialLayout;
		VkShaderModule _vertModule;
		VkShaderModule _fragModule;
	};
};