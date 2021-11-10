#pragma once
#include "ShaderSet.h"
#include "VkRenderer/BindingInfo.h"

struct Camera2d final
{
	glm::vec3 position{};
	float aspectRatio = 1;

	struct Frame final
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorSet descriptor;
	};

	class System final : public ShaderSet<Camera2d, Frame>
	{
	public:
		typedef Singleton<System> Instance;

		explicit System(uint32_t size);
		void Cleanup() override;
		void Update() override;

		[[nodiscard]] VkDescriptorSetLayout GetLayout() const;

	private:
		void ConstructInstanceFrame(Frame& frame, Camera2d& material, uint32_t denseId) override;
		void CleanupInstanceFrame(Frame& frame, Camera2d& material, uint32_t denseId) override;

		vi::BindingInfo _bindingInfo{};
		VkDescriptorSetLayout _descriptorLayout;
		VkDescriptorPool _descriptorPool;
	};
};
