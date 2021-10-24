#pragma once
#include "Debugger.h"
#include "PhysicalDeviceFactory.h"
#include "Queues.h"
#include "SwapChain.h"
#include "Pipeline.h"

namespace vi
{
	class WindowSystem;

	class VkRenderer final
	{
	public:
		struct Settings final
		{
			PhysicalDeviceFactory::Settings physicalDevice{};
			Debugger::Settings debugger{};

			std::vector<const char*> deviceExtensions =
			{
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
		};

		std::unique_ptr<Settings> settings{};

		WindowSystem& windowSystem;
		Debugger debugger{};
		SwapChain swapChain{};

		VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		Queues queues;
		VkCommandPool commandPool;

		explicit VkRenderer(class WindowSystem& system, const Settings& settings = {});
		~VkRenderer();

		[[nodiscard]] VkShaderModule CreateShaderModule(const std::vector<char>& data) const;
		void DestroyShaderModule(VkShaderModule module) const;

		[[nodiscard]] VkRenderPass CreateRenderPass(const struct RenderPassInfo& info) const;
		void DestroyRenderPass(VkRenderPass renderPass) const;

		[[nodiscard]] VkDescriptorSetLayout CreateLayout(const struct DescriptorLayoutInfo& info) const;
		void DestroyLayout(VkDescriptorSetLayout layout) const;

		[[nodiscard]] Pipeline CreatePipeline(const struct PipelineLayout& info) const;
		void DestroyPipeline(Pipeline pipeline) const;

		[[nodiscard]] VkCommandBuffer CreateCommandBuffer() const;
		void DestroyCommandBuffer(VkCommandBuffer commandBuffer) const;

		[[nodiscard]] VkImageView CreateImageView(VkImage image, VkFormat format) const;
		void DestroyImageView(VkImageView imageView) const;

		[[nodiscard]] VkFramebuffer CreateFrameBuffer(VkImageView imageView, VkRenderPass renderPass) const;
		void DestroyFrameBuffer(VkFramebuffer frameBuffer) const;

		[[nodiscard]] VkSemaphore CreateSemaphore() const;
		void DestroySemaphore(VkSemaphore semaphore) const;

		[[nodiscard]] VkFence CreateFence() const;
		void DestroyFence(VkFence fence) const;

		void BeginCommandBufferRecording(VkCommandBuffer commandBuffer);
		void EndCommandBufferRecording(VkCommandBuffer commandBuffer);

		void BeginRenderPass(VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer, 
			VkRenderPass renderPass, glm::ivec2 offset, glm::ivec2 extent);
		void EndRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass);

		VkSubmitInfo Submit(VkCommandBuffer* buffers, uint32_t buffersCount, 
			VkSemaphore waitSemaphore, VkSemaphore signalSemaphore) const;

		void Rebuild();

	private:
		VkPipelineStageFlags _waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		void CreateSwapChainDependencies();
		void CleanupSwapChainDependendies();
	};
}
