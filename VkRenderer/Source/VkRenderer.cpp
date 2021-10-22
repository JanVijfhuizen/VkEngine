#include "pch.h"
#include "VkRenderer.h"
#include "WindowSystem.h"
#include "InstanceFactory.h"
#include "LogicalDeviceFactory.h"
#include "PipelineInfo.h"

namespace vi
{
	VkRenderer::VkRenderer(WindowSystem& system, const Settings& settings) :  _windowSystem(system)
	{
		_settings = std::make_unique<Settings>();
		*_settings = settings;

		const InstanceFactory::Info instanceInfo
		{
			_windowSystem,
			_debugger,
			_instance
		};
		InstanceFactory{instanceInfo};

		_debugger.Construct(_settings->debugger, _instance);
		_windowSystem.CreateSurface(_instance, _surface);

		const PhysicalDeviceFactory::Info physicalDeviceInfo
		{
			_settings->physicalDevice,
			_settings->deviceExtensions,
			_instance,
			_surface,
			_physicalDevice,
		};
		PhysicalDeviceFactory{physicalDeviceInfo};

		const LogicalDeviceFactory::Info logicalDeviceInfo
		{
			_settings->deviceExtensions,
			_physicalDevice,
			_surface,
			_debugger,
			_device,
			_queues
		};
		LogicalDeviceFactory{logicalDeviceInfo};

		CreateSwapChainDependencies();
	}

	VkRenderer::~VkRenderer()
	{
		CleanupSwapChainDependendies();

		vkDestroyDevice(_device, nullptr);
		_debugger.Cleanup();

		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyInstance(_instance, nullptr);
	}

	VkShaderModule VkRenderer::CreateShaderModule(const std::vector<char>& data) const
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = data.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

		VkShaderModule vkModule;
		const auto result = vkCreateShaderModule(_device, &createInfo, nullptr, &vkModule);
		assert(!result);

		return vkModule;
	}

	void VkRenderer::DestroyShaderModule(const VkShaderModule module) const
	{
		vkDestroyShaderModule(_device, module, nullptr);
	}

	VkPipeline VkRenderer::CreatePipeline(const PipelineInfo& info)
	{
		std::vector<VkPipelineShaderStageCreateInfo> modules{};
		
		const uint32_t modulesCount = info.modules.size();
		modules.resize(modulesCount);

		for (uint32_t i = 0; i < modulesCount; ++i)
		{
			const auto& moduleInfo = info.modules[i];
			auto& vertShaderStageInfo = modules[i];

			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = moduleInfo.flags;
			vertShaderStageInfo.module = moduleInfo.module;
			vertShaderStageInfo.pName = "main";
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(info.attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &info.bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = info.attributeDescriptions.data();

		return {};
	}

	void VkRenderer::DestroyPipeline(const VkPipeline pipeline)
	{
	}

	void VkRenderer::Rebuild()
	{
		CleanupSwapChainDependendies();
		CreateSwapChainDependencies();
	}

	void VkRenderer::CreateSwapChainDependencies()
	{
		const SwapChain::Info swapChainInfo
		{
			_physicalDevice,
			_surface,
			_device,
			&_windowSystem
		};

		_swapChain.Construct(swapChainInfo);
	}

	void VkRenderer::CleanupSwapChainDependendies()
	{
		_swapChain.Cleanup();
	}
}
