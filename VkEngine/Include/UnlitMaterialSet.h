#pragma once
#include "MaterialSet.h"
#include "VkRenderer/BindingInfo.h"

struct UnlitMaterial final
{
	struct Frame final
	{
		union
		{
			struct
			{
				VkDescriptorSet camSet;
				VkDescriptorSet matSet;
			};
			VkDescriptorSet sets[2];
		};

		VkSampler matDiffuseSampler;
	};

	class System final : public MaterialSet<UnlitMaterial, Frame>
	{
	public:
		explicit System(uint32_t size);
		void Cleanup();

		void ConstructInstance(uint32_t denseId) override;
		void CleanupInstance(uint32_t denseId) override;

		void Update() override;

	private:
		vi::BindingInfo _camBinding{};
		vi::Pipeline _pipeline;
		VkDescriptorSetLayout _camLayout;
		VkDescriptorSetLayout _materialLayout;
		VkShaderModule _vertModule;
		VkShaderModule _fragModule;
		VkDescriptorPool _uboPool;
	};

	Texture* diffuseTexture = nullptr;
};
