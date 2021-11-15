#pragma once
#include "DepthBuffer.h"
#include "ShaderSet.h"
#include "DescriptorPool.h"

struct Light3d final
{
	struct Frame final 
	{
		DepthBuffer depthBuffer;
		VkFramebuffer frameBuffer;
	};

	class System final : public ShaderSet<Light3d, Frame>
	{
	public:
		typedef Singleton<System> Instance;

		struct Info final
		{
			glm::ivec2 shadowResolution{800, 600};
		};

		explicit System(uint32_t size, const Info& info = {});
		void Cleanup() override;

		void Update() override;

	private:
		const Info _info;
		VkRenderPass _renderPass;
		VkShaderModule _vertModule;
		vi::Pipeline _pipeline;
		DescriptorPool _descriptorPool;

		void ConstructInstanceFrame(Frame& frame, Light3d& material, uint32_t denseId) override;
		void CleanupInstanceFrame(Frame& frame, Light3d& material, uint32_t denseId) override;
	};
};

