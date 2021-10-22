#include "pch.h"
#include "VkRenderer.h"
#include "WindowSystem.h"
#include "InstanceFactory.h"
#include "LogicalDeviceFactory.h"
#include "PipelineInfo.h"
#include "RenderPassInfo.h"
#include "DescriptorLayoutInfo.h"

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

	VkDescriptorSetLayout VkRenderer::CreateLayout(const DescriptorLayoutInfo& info) const
	{
		const uint32_t bindingsCount = info.bindings.size();
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings{};
		layoutBindings.resize(bindingsCount);

		for (uint32_t i = 0; i < bindingsCount; ++i)
		{
			const auto& binding = info.bindings[i];
			auto& uboLayoutBinding = layoutBindings[i];

			uboLayoutBinding.binding = i;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = binding.flag;
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.bindingCount = bindingsCount;
		layoutInfo.pBindings = layoutBindings.data();

		VkDescriptorSetLayout layout;
		const auto result = vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &layout);
		assert(!result);
		return layout;
	}

	void VkRenderer::DestroyLayout(const VkDescriptorSetLayout layout) const
	{
		vkDestroyDescriptorSetLayout(_device, layout, nullptr);
	}

	Pipeline VkRenderer::CreatePipeline(const PipelineLayout& info) const
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

		// Todo multisampling.
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		if (info.colorBlendingEnabled)
			colorBlendAttachment = info.colorBlending;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = info.colorBlendingEnabled;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		for (auto& blendConstant : colorBlending.blendConstants)
			blendConstant = 0.0f;

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 0;
		dynamicState.pDynamicStates = nullptr;

		std::vector<VkPushConstantRange> pushConstantRanges{};
		for (const auto& pushConstant : info.pushConstants)
		{
			VkPushConstantRange pushConstantRange;
			pushConstantRange.offset = 0;
			pushConstantRange.size = pushConstant.size;
			pushConstantRange.stageFlags = pushConstant.flag;
			pushConstantRanges.push_back(pushConstantRange);
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = info.setLayouts.size();
		pipelineLayoutInfo.pSetLayouts = info.setLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = info.pushConstants.size();
		pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

		VkPipelineLayout pipelineLayout;
		auto result = vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
		assert(!result);
		
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = modulesCount;
		pipelineInfo.pStages = modules.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		// Todo add depth stencil.
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = info.renderPass;
		pipelineInfo.subpass = 0;
		// Todo optimize with base pipeline.
		pipelineInfo.basePipelineHandle = info.basePipeline;
		pipelineInfo.basePipelineIndex = info.basePipelineIndex;

		VkPipeline graphicsPipeline;
		result = vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
		assert(!result);

		return
		{
			graphicsPipeline,
			pipelineLayout 
		};
	}

	void VkRenderer::DestroyPipeline(const Pipeline pipeline) const
	{
		vkDestroyPipeline(_device, pipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(_device, pipeline.layout, nullptr);
	}

	void VkRenderer::AssignSwapChainRenderPass(const VkRenderPass renderPass)
	{
		_swapChainRenderPass = renderPass;
		_swapChain.CleanupFrameBuffers();
		_swapChain.CreateFrameBuffers(renderPass);
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
