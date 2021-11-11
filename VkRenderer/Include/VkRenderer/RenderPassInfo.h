#pragma once

namespace vi
{
	struct RenderPassInfo final
	{
		struct Attachment final
		{
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		};

		VkFormat colorFormat;
		std::vector<Attachment> colorAttachments{};

		bool useDepthAttachment = false;
		VkFormat depthFormat;
		VkAttachmentStoreOp depthStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	};
}
