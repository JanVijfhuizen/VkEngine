#pragma once
#include "ShaderSet.h"
#include "DescriptorPool.h"

struct UnlitMaterial3d final
{
	struct Frame final
	{
		VkDescriptorSet descriptorSet;
		VkSampler matDiffuseSampler;
	};

	class System final : public ShaderSet<UnlitMaterial3d, Frame>
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

		void ConstructInstanceFrame(Frame& frame, UnlitMaterial3d& material, uint32_t denseId) override;
		void CleanupInstanceFrame(Frame& frame, UnlitMaterial3d& material, uint32_t denseId) override;
	};

	Texture* diffuseTexture = nullptr;
};
