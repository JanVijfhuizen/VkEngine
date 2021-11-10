#pragma once
#include "ShaderSet.h"
#include "DescriptorPool.h"

struct UnlitMaterial2d final
{
	struct Frame final
	{
		VkDescriptorSet descriptorSet;
		VkSampler matDiffuseSampler;
	};

	class System final : public ShaderSet<UnlitMaterial2d, Frame>
	{
	public:
		typedef Singleton<System> Instance;

		explicit System(uint32_t size);
		void Cleanup() override;
		void Update() override;

	private:
		vi::Pipeline _pipeline;
		VkShaderModule _vertModule;
		VkShaderModule _fragModule;
		DescriptorPool _descriptorPool;

		void ConstructInstanceFrame(Frame& frame, UnlitMaterial2d& material, uint32_t denseId) override;
		void CleanupInstanceFrame(Frame& frame, UnlitMaterial2d& material, uint32_t denseId) override;
	};

	Texture* diffuseTexture = nullptr;
};
