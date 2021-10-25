#pragma once
#include "Debugger.h"
#include "PhysicalDeviceFactory.h"
#include "Queues.h"
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

		[[nodiscard]] Pipeline CreatePipeline(const struct PipelineLayoutInfo& info) const;
		void DestroyPipeline(Pipeline pipeline) const;

		[[nodiscard]] VkCommandBuffer CreateCommandBuffer() const;
		void DestroyCommandBuffer(VkCommandBuffer commandBuffer) const;

		[[nodiscard]] VkImageView CreateImageView(VkImage image, VkFormat format) const;
		void DestroyImageView(VkImageView imageView) const;

		[[nodiscard]] VkFramebuffer CreateFrameBuffer(VkImageView imageView, VkRenderPass renderPass, VkExtent2D extent) const;
		void DestroyFrameBuffer(VkFramebuffer frameBuffer) const;

		[[nodiscard]] VkSemaphore CreateSemaphore() const;
		void DestroySemaphore(VkSemaphore semaphore) const;

		[[nodiscard]] VkFence CreateFence() const;
		void DestroyFence(VkFence fence) const;

		template <typename T>
		[[nodiscard]] VkBuffer CreateBuffer(uint32_t vertCount, VkBufferUsageFlags flags) const;
		void DestroyBuffer(VkBuffer buffer);

		[[nodiscard]] VkDeviceMemory AllocateMemory(VkBuffer buffer) const;
		void BindMemory(VkBuffer buffer, VkDeviceMemory memory);
		void FreeMemory(VkDeviceMemory memory);
		template <typename T>
		void MapMemory(VkDeviceMemory memory, T* input, VkDeviceSize offset, VkDeviceSize size);

		void BeginCommandBufferRecording(VkCommandBuffer commandBuffer);
		void EndCommandBufferRecording(VkCommandBuffer commandBuffer);

		void BeginRenderPass(VkCommandBuffer commandBuffer, VkFramebuffer frameBuffer, 
			VkRenderPass renderPass, glm::ivec2 offset, glm::ivec2 extent);
		void EndRenderPass(VkCommandBuffer commandBuffer);

		void BindPipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline);
		void BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkDescriptorSet* sets, uint32_t setCount);

		void BindVertexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer);
		void BindIndicesBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer);

		template <typename T>
		void UpdatePushConstant(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkFlags flag, const T& input);

		void Draw(VkCommandBuffer commandBuffer, uint32_t indexCount);
		void Submit(VkCommandBuffer* buffers, uint32_t buffersCount, 
			VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence) const;

		[[nodiscard]] uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

		void DeviceWaitIdle() const;
	};

	template <typename T>
	VkBuffer VkRenderer::CreateBuffer(const uint32_t vertCount, const VkBufferUsageFlags flags) const
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(T) * vertCount;
		bufferInfo.usage = flags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer vertexBuffer;
		const auto result = vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer);
		assert(!result);
		return vertexBuffer;
	}

	template <typename T>
	void VkRenderer::MapMemory(const VkDeviceMemory memory, T* input, const VkDeviceSize offset, const VkDeviceSize size)
	{
		void* data;
		vkMapMemory(device, memory, offset, size, 0, &data);
		memcpy(data, input, static_cast<size_t>(size));
		vkUnmapMemory(device, memory);
	}

	template <typename T>
	void VkRenderer::UpdatePushConstant(const VkCommandBuffer commandBuffer, 
		const VkPipelineLayout layout, const VkFlags flag, const T& input)
	{
		vkCmdPushConstants(commandBuffer, layout, flag, 0, sizeof(T), &input);
	}
}
