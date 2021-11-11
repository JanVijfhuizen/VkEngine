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
	void VkRenderer::Construct(WindowSystem& system, const Settings& settings)
	{
		_windowSystem = &system;
		_settings = std::make_unique<Settings>();
		*_settings = settings;

		_debugger = Debugger{ *this };

		InstanceFactory{ *this };

		_debugger.Construct();
		_windowSystem->CreateSurface(_instance, _surface);

		PhysicalDeviceFactory{ *this, settings.physicalDevice };
		LogicalDeviceFactory{ *this };
		CommandPoolFactory{ *this };
	}

	void VkRenderer::Cleanup()
	{
		DeviceWaitIdle();

		CommandPoolFactory::Cleanup(*this);
		LogicalDeviceFactory::Cleanup(*this);
		_debugger.Cleanup();
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		InstanceFactory::Cleanup(*this);
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
			uboLayoutBinding.descriptorType = binding.type;
			uboLayoutBinding.descriptorCount = binding.count;
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

	VkDescriptorPool VkRenderer::CreateDescriptorPool(VkDescriptorType* types, const uint32_t typeCount, const uint32_t maxSets) const
	{
		std::vector<VkDescriptorPoolSize> sizes{};
		sizes.resize(typeCount);

		for (uint32_t i = 0; i < typeCount; ++i)
		{
			auto& size = sizes[i];
			size.type = types[i];
			size.descriptorCount = maxSets;
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = typeCount;
		poolInfo.pPoolSizes = sizes.data();
		poolInfo.maxSets = maxSets;

		VkDescriptorPool pool;
		const auto result = vkCreateDescriptorPool(_device, &poolInfo, nullptr, &pool);
		assert(!result);
		return pool;
	}

	void VkRenderer::CreateDescriptorSets(const VkDescriptorPool pool, const VkDescriptorSetLayout layout, 
		VkDescriptorSet* outSets, const uint32_t setCount) const
	{
		std::vector<VkDescriptorSetLayout> layouts(setCount, layout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = setCount;
		allocInfo.pSetLayouts = layouts.data();

		const auto result = vkAllocateDescriptorSets(_device, &allocInfo, outSets);
		assert(!result);
	}

	void VkRenderer::BindBuffer(const VkDescriptorSet set, const VkBuffer buffer, const BindingInfo& info, 
		const uint32_t bindingIndex, const uint32_t arrayIndex) const
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = info.size;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = set;
		descriptorWrite.dstBinding = bindingIndex;
		descriptorWrite.dstArrayElement = arrayIndex;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);
	}

	void VkRenderer::BindSampler(const VkDescriptorSet set, const VkImageView imageView, const VkSampler sampler,
		const uint32_t bindingIndex, const uint32_t arrayIndex) const
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = imageView;
		imageInfo.sampler = sampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = set;
		descriptorWrite.dstBinding = bindingIndex;
		descriptorWrite.dstArrayElement = arrayIndex;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);
	}

	void VkRenderer::DestroyDescriptorPool(const VkDescriptorPool pool) const
	{
		vkDestroyDescriptorPool(_device, pool, nullptr);
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

	VkCommandBuffer VkRenderer::CreateCommandBuffer() const
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = _commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		const auto result = vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);
		assert(!result);

		return commandBuffer;
	}

	void VkRenderer::DestroyCommandBuffer(const VkCommandBuffer commandBuffer) const
	{
		vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
	}

	VkImage VkRenderer::CreateImage(const glm::ivec2 resolution, const VkFormat format,
		const VkImageTiling tiling, const VkImageUsageFlags usage) const
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(resolution.x);
		imageInfo.extent.height = static_cast<uint32_t>(resolution.y);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkImage image;
		const auto result = vkCreateImage(_device, &imageInfo, nullptr, &image);
		assert(!result);
		return image;
	}

	void VkRenderer::DestroyImage(const VkImage image) const
	{
		vkDestroyImage(_device, image, nullptr);
	}

	VkImageView VkRenderer::CreateImageView(const VkImage image, const VkFormat format,
		const VkImageAspectFlags aspectFlags) const
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

		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		const auto result = vkCreateImageView(_device, &createInfo, nullptr, &imageView);
		assert(!result);
		return imageView;
	}

	void VkRenderer::DestroyImageView(const VkImageView imageView) const
	{
		vkDestroyImageView(_device, imageView, nullptr);
	}

	VkSampler VkRenderer::CreateSampler(const VkFilter magFilter, const VkFilter minFilter) const
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(_physicalDevice, &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkSampler sampler;
		const auto result = vkCreateSampler(_device, &samplerInfo, nullptr, &sampler);
		assert(!result);
		return sampler;
	}

	void VkRenderer::DestroySampler(const VkSampler sampler) const
	{
		vkDestroySampler(_device, sampler, nullptr);
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
		const auto result = vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &frameBuffer);
		assert(!result);
		return frameBuffer;
	}

	void VkRenderer::DestroyFrameBuffer(const VkFramebuffer frameBuffer) const
	{
		vkDestroyFramebuffer(_device, frameBuffer, nullptr);
	}

	VkSemaphore VkRenderer::CreateSemaphore() const
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkSemaphore semaphore;
		const auto result = vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &semaphore);
		assert(!result);
		return semaphore;
	}

	void VkRenderer::DestroySemaphore(const VkSemaphore semaphore) const
	{
		vkDestroySemaphore(_device, semaphore, nullptr);
	}

	VkFence VkRenderer::CreateFence() const
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkFence fence;
		const auto result = vkCreateFence(_device, &fenceInfo, nullptr, &fence);
		assert(!result);
		return fence;
	}

	void VkRenderer::DestroyFence(const VkFence fence) const
	{
		vkDestroyFence(_device, fence, nullptr);
	}

	VkBuffer VkRenderer::CreateBuffer(const VkDeviceSize size, const VkBufferUsageFlags flags) const
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = flags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer vertexBuffer;
		const auto result = vkCreateBuffer(_device, &bufferInfo, nullptr, &vertexBuffer);
		assert(!result);
		return vertexBuffer;
	}

	void VkRenderer::DestroyBuffer(const VkBuffer buffer) const
	{
		vkDestroyBuffer(_device, buffer, nullptr);
	}

	VkDeviceMemory VkRenderer::AllocateMemory(const VkImage image, const VkMemoryPropertyFlags flags) const
	{
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(_device, image, &memRequirements);
		return AllocateMemory(memRequirements, flags);
	}

	void VkRenderer::WaitForFence(const VkFence fence) const
	{
		vkWaitForFences(_device, 1, &fence, VK_TRUE, UINT64_MAX);
	}

	VkDeviceMemory VkRenderer::AllocateMemory(const VkBuffer buffer, const VkMemoryPropertyFlags flags) const
	{
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);

		return AllocateMemory(memRequirements, flags);
	}

	VkDeviceMemory VkRenderer::AllocateMemory(const VkMemoryRequirements memRequirements, const VkMemoryPropertyFlags flags) const
	{
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, flags);

		VkDeviceMemory memory;
		const auto result = vkAllocateMemory(_device, &allocInfo, nullptr, &memory);
		assert(!result);

		return memory;
	}

	void VkRenderer::BindMemory(const VkImage image, const VkDeviceMemory memory) const
	{
		vkBindImageMemory(_device, image, memory, 0);
	}

	void VkRenderer::BindMemory(const VkBuffer buffer, const VkDeviceMemory memory) const
	{
		vkBindBufferMemory(_device, buffer, memory, 0);
	}

	void VkRenderer::FreeMemory(const VkDeviceMemory memory) const
	{
		vkFreeMemory(_device, memory, nullptr);
	}

	void VkRenderer::CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size, 
		const VkDeviceSize srcOffset, const VkDeviceSize dstOffset) const
	{
		VkBufferCopy region{};
		region.srcOffset = srcOffset;
		region.dstOffset = dstOffset;
		region.size = size;
		vkCmdCopyBuffer(_currentCommandBuffer, srcBuffer, dstBuffer, 1, &region);
	}

	void VkRenderer::CopyBuffer(const VkBuffer srcBuffer, const VkImage dstImage, const uint32_t width, const uint32_t height) const
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent =
		{
			width,
			height,
			1
		};

		auto& subResource = region.imageSubresource;
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.mipLevel = 0;
		subResource.baseArrayLayer = 0;
		subResource.layerCount = 1;

		vkCmdCopyBufferToImage(_currentCommandBuffer, srcBuffer, dstImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	void VkRenderer::TransitionImageLayout(const VkImage image, const VkImageLayout oldLayout, const VkImageLayout newLayout) const
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
			throw std::exception("Layout transition not supported!");

		vkCmdPipelineBarrier(_currentCommandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void VkRenderer::BeginCommandBufferRecording(const VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		_currentCommandBuffer = commandBuffer;
	}

	void VkRenderer::EndCommandBufferRecording() const
	{
		const auto result = vkEndCommandBuffer(_currentCommandBuffer);
		assert(!result);
	}

	void VkRenderer::BeginRenderPass(const VkFramebuffer frameBuffer, 
		const VkRenderPass renderPass, const glm::ivec2 offset, const glm::ivec2 extent) const
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

		vkCmdBeginRenderPass(_currentCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VkRenderer::EndRenderPass() const
	{
		vkCmdEndRenderPass(_currentCommandBuffer);
	}

	void VkRenderer::BindPipeline(const Pipeline pipeline)
	{
		_currentPipeline = pipeline;
		vkCmdBindPipeline(_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
	}

	void VkRenderer::BindDescriptorSets(VkDescriptorSet* sets, const uint32_t setCount) const
	{
		vkCmdBindDescriptorSets(_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
			_currentPipeline.layout, 0, setCount, sets, 0, nullptr);
	}

	void VkRenderer::BindVertexBuffer(const VkBuffer buffer) const
	{
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(_currentCommandBuffer, 0, 1, &buffer, &offset);
	}

	void VkRenderer::BindIndicesBuffer(const VkBuffer buffer) const
	{
		vkCmdBindIndexBuffer(_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT16);
	}

	void VkRenderer::Draw(const uint32_t indexCount) const
	{
		vkCmdDrawIndexed(_currentCommandBuffer, indexCount, 1, 0, 0, 0);
	}

	void VkRenderer::Submit(VkCommandBuffer* buffers, const uint32_t buffersCount,
		const VkSemaphore waitSemaphore, const VkSemaphore signalSemaphore, const VkFence fence) const
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.waitSemaphoreCount = waitSemaphore ? 1 : 0;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = buffersCount;
		submitInfo.pCommandBuffers = buffers;
		submitInfo.signalSemaphoreCount = signalSemaphore ? 1 : 0;
		submitInfo.pSignalSemaphores = &signalSemaphore;

		vkResetFences(_device, 1, &fence);
		const auto result = vkQueueSubmit(_queues.graphics, 1, &submitInfo, fence);
		assert(!result);
	}

	uint32_t VkRenderer::FindMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) const
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			if (typeFilter & 1 << i)
			{
				const bool requiredPropertiesPresent = (memProperties.memoryTypes[i].propertyFlags & properties) == properties;
				if (!requiredPropertiesPresent)
					continue;

				return i;
			}

		throw std::exception("Memory type not available!");
	}

	void VkRenderer::DeviceWaitIdle() const
	{
		const auto result = vkDeviceWaitIdle(_device);
		assert(!result);
	}

	VkFormat VkRenderer::FindSupportedFormat(const std::vector<VkFormat>& candidates, 
		const VkImageTiling tiling, const VkFormatFeatureFlags features) const
	{
		for (VkFormat format : candidates) 
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				return format;
			if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				return format;
		}

		throw std::exception("Format not available!");
	}
}
