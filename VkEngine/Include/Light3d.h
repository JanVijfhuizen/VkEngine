#pragma once
#include "DepthBuffer.h"
#include "ShaderSet.h"
#include "DescriptorPool.h"
#include "VkRenderer/BindingInfo.h"

struct alignas(4) Light3d final
{
	struct Frame final 
	{
		DepthBuffer depthBuffer;
		VkFramebuffer frameBuffer;

		VkDescriptorSet descriptorSet;
		VkBuffer lightBuffer;
		VkDeviceMemory lightMemory;
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
		struct Ubo final
		{
			glm::mat4 lightSpaceMatrix;
		};

		const Info _info;
		VkRenderPass _renderPass;
		VkShaderModule _vertModule;
		vi::Pipeline _pipeline;
		VkCommandBuffer _commandBuffer;
		VkFence _fence;
		DescriptorPool _descriptorPool;
		vi::BindingInfo _bindingInfo;

		void ConstructInstanceFrame(Frame& frame, Light3d& material, uint32_t denseId) override;
		void CleanupInstanceFrame(Frame& frame, Light3d& material, uint32_t denseId) override;
	};
};
