#pragma once
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
		typedef Singleton<System> Instance;

		explicit System(uint32_t size);
		void Cleanup() override;
		void Update() override;

	private:
		void ConstructInstanceFrame(Frame& frame, UnlitMaterial& material, uint32_t denseId) override;
		void CleanupInstanceFrame(Frame& frame, UnlitMaterial& material, uint32_t denseId) override;
	};

	Texture* diffuseTexture = nullptr;
};
