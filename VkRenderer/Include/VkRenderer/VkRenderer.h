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

		void AssignSwapChainRenderPass(VkRenderPass renderPass);

		void Rebuild();

	private:
		std::unique_ptr<Settings> _settings{};

		WindowSystem& _windowSystem;
		Debugger _debugger{};
		SwapChain _swapChain{};

		VkInstance _instance;
		VkSurfaceKHR _surface;
		VkPhysicalDevice _physicalDevice;
		VkDevice _device;
		Queues _queues;

		VkRenderPass _swapChainRenderPass;

		void CreateSwapChainDependencies();
		void CleanupSwapChainDependendies();
	};
}
