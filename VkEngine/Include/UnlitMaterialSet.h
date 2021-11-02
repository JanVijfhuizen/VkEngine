#pragma once
#include "VkRenderer/BindingInfo.h"
#include "ShaderSet.h"

struct UnlitMaterial final
{
	struct Frame final
	{
		VkDescriptorSet descriptorSet;
		VkSampler matDiffuseSampler;
	};

	class System final : public ShaderSet<UnlitMaterial, Frame>
	{
	public:
		explicit System(uint32_t size);
		void Cleanup();
		void Update() override;

	private:
		vi::Pipeline _pipeline;
		VkDescriptorSetLayout _materialLayout;
		VkShaderModule _vertModule;
		VkShaderModule _fragModule;
		VkDescriptorPool _uboPool;

		void ConstructInstanceFrame(Frame& frame, UnlitMaterial& material, uint32_t denseId) override;
		void CleanupInstanceFrame(Frame& frame, UnlitMaterial& material, uint32_t denseId) override;
	};

	Texture* diffuseTexture = nullptr;
};
