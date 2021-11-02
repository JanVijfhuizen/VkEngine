#pragma once
#include "ShaderSet.h"
#include "VkRenderer/BindingInfo.h"

struct Camera final
{
	glm::vec3 position;
	float aspectRatio;

	struct Frame final
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorSet descriptor;
	};

	class System final : public ShaderSet<Camera, Frame>
	{
	public:
		explicit System(uint32_t size);
		void Update() override;

	private:
		void ConstructInstanceFrame(Frame& frame, Camera& material, uint32_t denseId) override;
		void CleanupInstanceFrame(Frame& frame, Camera& material, uint32_t denseId) override;

	private:
		vi::BindingInfo _bindingInfo{};
		VkDescriptorSetLayout _descriptorLayout;
		VkDescriptorPool _descriptorPool;
	};
};
