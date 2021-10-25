#include "pch.h"
#include "VkRenderer.h"
#include "WindowSystem.h"
#include "InstanceFactory.h"
#include "LogicalDeviceFactory.h"
#include "PipelineInfo.h"
#include "RenderPassInfo.h"
#include "DescriptorLayoutInfo.h"
#include "CommandPoolFactory.h"

namespace vi
{
	VkRenderer::VkRenderer(WindowSystem& system, const Settings& settings) :  windowSystem(system)
	{
		this->settings = std::make_unique<Settings>();
		*this->settings = settings;

		debugger = Debugger{ *this };

		InstanceFactory{*this};

		debugger.Construct();
		windowSystem.CreateSurface(instance, surface);

		PhysicalDeviceFactory{*this, settings.physicalDevice};
		LogicalDeviceFactory{*this};
		CommandPoolFactory{*this};
	}

	VkRenderer::~VkRenderer()
	{
		DeviceWaitIdle();

		CommandPoolFactory::Cleanup(*this);
		LogicalDeviceFactory::Cleanup(*this);
		debugger.Cleanup();
		vkDestroySurfaceKHR(instance, surface, nullptr);
		InstanceFactory::Cleanup(*this);
	}

	VkShaderModule VkRenderer::CreateShaderModule(const std::vector<char>& data) const
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = data.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

		VkShaderModule vkModule;
		const auto result = vkCreateShaderModule(device, &createInfo, nullptr, &vkModule);
		assert(!result);

		return vkModule;
	}

	void VkRenderer::DestroyShaderModule(const VkShaderModule module) const
	{
		vkDestroyShaderModule(device, module, nullptr);
	}

	VkRenderPass VkRenderer::CreateRenderPass(const RenderPassInfo& info) const
	{
		const uint32_t attachmentsCount = info.attachments.size();
		const auto& format = info.format;

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

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachmentsCount;
		renderPassInfo.pAttachments = descriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkRenderPass renderPass;
		const auto result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
		assert(!result);
		return renderPass;
	}

	void VkRenderer::DestroyRenderPass(const VkRenderPass renderPass) const
	{
		vkDestroyRenderPass(device, renderPass, nullptr);
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
		const auto result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout);
		assert(!result);
		return layout;
	}

	void VkRenderer::DestroyLayout(const VkDescriptorSetLayout layout) const
	{
		vkDestroyDescriptorSetLayout(device, layout, nullptr);
	}

	Pipeline VkRenderer::CreatePipeline(const PipelineLayoutInfo& info) const
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

		const auto& extent = info.extent;

		VkViewport viewport{};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

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

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

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
		auto result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
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
		pipelineInfo.basePipelineHandle = info.basePipeline;
		pipelineInfo.basePipelineIndex = info.basePipelineIndex;

		VkPipeline graphicsPipeline;
		result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
		assert(!result);

		return
		{
			graphicsPipeline,
			pipelineLayout 
		};
	}

	void VkRenderer::DestroyPipeline(const Pipeline pipeline) const
	{
		vkDestroyPipeline(device, pipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipeline.layout, nullptr);
	}

	VkCommandBuffer VkRenderer::CreateCommandBuffer() const
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		const auto result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
		assert(!result);

		return commandBuffer;
	}

	void VkRenderer::DestroyCommandBuffer(const VkCommandBuffer commandBuffer) const
	{
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	VkImageView VkRenderer::CreateImageView(const VkImage image, const VkFormat format) const
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		const auto result = vkCreateImageView(device, &createInfo, nullptr, &imageView);
		assert(!result);
		return imageView;
	}

	void VkRenderer::DestroyImageView(const VkImageView imageView) const
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

	VkFramebuffer VkRenderer::CreateFrameBuffer(const VkImageView imageView, const VkRenderPass renderPass, const VkExtent2D extent) const
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &imageView;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		VkFramebuffer frameBuffer;
		const auto result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &frameBuffer);
		assert(!result);
		return frameBuffer;
	}

	void VkRenderer::DestroyFrameBuffer(const VkFramebuffer frameBuffer) const
	{
		vkDestroyFramebuffer(device, frameBuffer, nullptr);
	}

	VkSemaphore VkRenderer::CreateSemaphore() const
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkSemaphore semaphore;
		const auto result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore);
		assert(!result);
		return semaphore;
	}

	void VkRenderer::DestroySemaphore(const VkSemaphore semaphore) const
	{
		vkDestroySemaphore(device, semaphore, nullptr);
	}

	VkFence VkRenderer::CreateFence() const
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkFence fence;
		const auto result = vkCreateFence(device, &fenceInfo, nullptr, &fence);
		assert(!result);
		return fence;
	}

	void VkRenderer::DestroyFence(const VkFence fence) const
	{
		vkDestroyFence(device, fence, nullptr);
	}

	void VkRenderer::DestroyBuffer(const VkBuffer buffer)
	{
		vkDestroyBuffer(device, buffer, nullptr);
	}

	VkDeviceMemory VkRenderer::AllocateMemory(const VkBuffer buffer) const
	{
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VkDeviceMemory memory;
		const auto result = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
		assert(!result);

		return memory;
	}

	void VkRenderer::BindMemory(const VkBuffer buffer, const VkDeviceMemory memory)
	{
		vkBindBufferMemory(device, buffer, memory, 0);
	}

	void VkRenderer::FreeMemory(const VkDeviceMemory memory)
	{
		vkFreeMemory(device, memory, nullptr);
	}

	void VkRenderer::BeginCommandBufferRecording(const VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
	}

	void VkRenderer::EndCommandBufferRecording(const VkCommandBuffer commandBuffer)
	{
		const auto result = vkEndCommandBuffer(commandBuffer);
		assert(!result);
	}

	void VkRenderer::BeginRenderPass(const VkCommandBuffer commandBuffer, const VkFramebuffer frameBuffer, 
		const VkRenderPass renderPass, const glm::ivec2 offset, const glm::ivec2 extent)
	{
		const VkExtent2D extentVk
		{
			static_cast<uint32_t>(extent.x),
			static_cast<uint32_t>(extent.y)
		};

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frameBuffer;
		renderPassInfo.renderArea.offset = {offset.x, offset.y};
		renderPassInfo.renderArea.extent = extentVk;

		VkClearValue clearColor = { {{0, 0, 0, 1}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VkRenderer::EndRenderPass(const VkCommandBuffer commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}

	void VkRenderer::BindPipeline(const VkCommandBuffer commandBuffer, const VkPipeline pipeline)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void VkRenderer::BindDescriptorSets(const VkCommandBuffer commandBuffer, const VkPipelineLayout layout,
		VkDescriptorSet* sets, const uint32_t setCount)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0,
			setCount, sets, 0, nullptr);
	}

	void VkRenderer::BindVertexBuffer(const VkCommandBuffer commandBuffer, const VkBuffer buffer)
	{
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offset);
	}

	void VkRenderer::BindIndicesBuffer(const VkCommandBuffer commandBuffer, const VkBuffer buffer)
	{
		vkCmdBindIndexBuffer(commandBuffer, buffer, 0, VK_INDEX_TYPE_UINT16);
	}

	void VkRenderer::Draw(const VkCommandBuffer commandBuffer, const uint32_t indexCount)
	{
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
	}

	void VkRenderer::Submit(VkCommandBuffer* buffers, const uint32_t buffersCount,
		const VkSemaphore waitSemaphore, const VkSemaphore signalSemaphore, const VkFence fence) const
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = buffersCount;
		submitInfo.pCommandBuffers = buffers;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;

		vkResetFences(device, 1, &fence);
		const auto result = vkQueueSubmit(queues.graphics, 1, &submitInfo, fence);
		assert(!result);
	}

	uint32_t VkRenderer::FindMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) const
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			if (typeFilter & 1 << i)
			{
				const bool requiredPropertiesPresent = (memProperties.memoryTypes[i].propertyFlags & properties) == properties;
				if (!requiredPropertiesPresent)
					continue;

				return i;
			}

		assert(false);
		return -1;
	}

	void VkRenderer::DeviceWaitIdle() const
	{
		const auto result = vkDeviceWaitIdle(device);
		assert(!result);
	}
}
