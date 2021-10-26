#pragma once
#include "Debugger.h"
#include "PhysicalDeviceFactory.h"
#include "Queues.h"
#include "Pipeline.h"

namespace vi
{
	class LogicalDeviceFactory;
	class InstanceFactory;
	class CommandPoolFactory;
	class SwapChain;
	class WindowSystem;

	class VkRenderer final
	{
		friend Debugger;
		friend SwapChain;

		friend CommandPoolFactory;
		friend InstanceFactory;
		friend LogicalDeviceFactory;
		friend PhysicalDeviceFactory;

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
		[[nodiscard]] VkBuffer CreateBuffer(uint32_t count, VkBufferUsageFlags flags) const;
		void DestroyBuffer(VkBuffer buffer);

		[[nodiscard]] VkDeviceMemory AllocateMemory(VkBuffer buffer) const;
		void BindMemory(VkBuffer buffer, VkDeviceMemory memory);
		void FreeMemory(VkDeviceMemory memory);
		template <typename T>
		void MapMemory(VkDeviceMemory memory, T* input, VkDeviceSize offset, uint32_t count);

		void BeginCommandBufferRecording(VkCommandBuffer commandBuffer);
		void EndCommandBufferRecording() const;

		void BeginRenderPass(VkFramebuffer frameBuffer, VkRenderPass renderPass, glm::ivec2 offset, glm::ivec2 extent);
		void EndRenderPass() const;

		void BindPipeline(VkPipeline pipeline) const;
		void BindDescriptorSets(VkPipelineLayout layout, VkDescriptorSet* sets, uint32_t setCount) const;

		void BindVertexBuffer(VkBuffer buffer) const;
		void BindIndicesBuffer(VkBuffer buffer) const;

		template <typename T>
		void UpdatePushConstant(VkPipelineLayout layout, VkFlags flag, const T& input);

		void Draw(uint32_t indexCount) const;
		void Submit(VkCommandBuffer* buffers, uint32_t buffersCount, 
			VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence) const;

		[[nodiscard]] uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

		void DeviceWaitIdle() const;

	private:
		std::unique_ptr<Settings> _settings{};

		WindowSystem& _windowSystem;
		Debugger _debugger{};

		VkInstance _instance;
		VkSurfaceKHR _surface;
		VkPhysicalDevice _physicalDevice;
		VkDevice _device;
		Queues _queues;
		VkCommandPool _commandPool;

		VkCommandBuffer _currentCommandBuffer;
	};

	template <typename T>
	VkBuffer VkRenderer::CreateBuffer(const uint32_t count, const VkBufferUsageFlags flags) const
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(T) * count;
		bufferInfo.usage = flags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer vertexBuffer;
		const auto result = vkCreateBuffer(_device, &bufferInfo, nullptr, &vertexBuffer);
		assert(!result);
		return vertexBuffer;
	}

	template <typename T>
	void VkRenderer::MapMemory(const VkDeviceMemory memory, T* input, const VkDeviceSize offset, const uint32_t count)
	{
		void* data;
		const uint32_t size = count * sizeof(T);
		vkMapMemory(_device, memory, offset, size, 0, &data);
		memcpy(data, input, size);
		vkUnmapMemory(_device, memory);
	}

	template <typename T>
	void VkRenderer::UpdatePushConstant(const VkPipelineLayout layout, const VkFlags flag, const T& input)
	{
		vkCmdPushConstants(_currentCommandBuffer, layout, flag, 0, sizeof(T), &input);
	}
}
