#include "pch.h"
#include "VkRenderer.h"
#include "WindowSystem.h"
#include "InstanceFactory.h"
#include "LogicalDeviceFactory.h"
#include "PipelineLayoutInfo.h"
#include "RenderPassInfo.h"

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

	VkRenderPass VkRenderer::CreateRenderPass(const RenderPassInfo& info) const
	{
		const uint32_t attachmentsCount = info.attachments.size();
		const auto format = _swapChain.GetFormat();

		std::vector<VkAttachmentDescription> descriptions{};
		descriptions.resize(info.attachments.size());

		for (uint32_t i = 0; i < attachmentsCount; ++i)
		{
			auto& descriptionInfo = info.attachments[i];
			auto& description = descriptions[i];

			description.format = format;
			description.samples = descriptionInfo.samples;
			description.loadOp = descriptionInfo.loadOp;
			description.storeOp = descriptionInfo.storeOp;
			description.stencilLoadOp = descriptionInfo.stencilLoadOp;
			description.stencilStoreOp = descriptionInfo.stencilStoreOp;
			description.initialLayout = descriptionInfo.initialLayout;
			description.finalLayout = descriptionInfo.finalLayout;
		}
		
		std::vector<VkAttachmentReference> colorAttachmentRefs{};
		colorAttachmentRefs.resize(attachmentsCount);

		for (uint32_t i = 0; i < attachmentsCount; ++i)
		{
			auto& attachmentRef = colorAttachmentRefs[i];
			attachmentRef.attachment = i;
			attachmentRef.layout = info.attachments[i].layout;
		}

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = attachmentsCount;
		subpass.pColorAttachments = colorAttachmentRefs.data();
		
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachmentsCount;
		renderPassInfo.pAttachments = descriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkRenderPass renderPass;
		const auto result = vkCreateRenderPass(_device, &renderPassInfo, nullptr, &renderPass);
		assert(!result);
		return renderPass;
	}

	void VkRenderer::DestroyRenderPass(const VkRenderPass renderPass) const
	{
		vkDestroyRenderPass(_device, renderPass, nullptr);
	}

	VkPipelineLayout VkRenderer::CreatePipelineLayout(const PipelineLayoutInfo& info) const
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

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = info.primitiveTopology;
		inputAssembly.primitiveRestartEnable = info.primitiveRestartEnable;

		const auto extent = _swapChain.GetExtent();

		VkViewport viewport{};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

		if (info.useViewport)
			viewport = info.viewport;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = info.depthClampEnable;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = info.polygonMode;
		rasterizer.lineWidth = info.lineWidth;
		rasterizer.cullMode = info.cullMode;
		rasterizer.frontFace = info.frontFace;
		rasterizer.depthBiasEnable = VK_FALSE;

		// TODO multisampling.
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		if (info.colorBlendingEnabled)
			colorBlendAttachment = info.colorBlending;

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 0;
		dynamicState.pDynamicStates = nullptr;

		// TODO LAYOUTS.
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VkPipelineLayout pipelineLayout;
		const auto result = vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
		assert(!result);
		return pipelineLayout;
	}

	void VkRenderer::DestroyPipelineLayout(const VkPipelineLayout layout) const
	{
		vkDestroyPipelineLayout(_device, layout, nullptr);
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
