#pragma once

namespace vi
{
	class Debugger final
	{
	public:
		void Construct(VkInstance instance);
		void Cleanup();

		[[nodiscard]] bool CheckValidationLayerSupport() const;
		[[nodiscard]] static VkDebugUtilsMessengerCreateInfoEXT CreateInfo();
		void EnableValidationLayers(VkDebugUtilsMessengerCreateInfoEXT& debugInfo, VkInstanceCreateInfo& instanceInfo) const;

	private:
		const std::vector<const char*> _validationLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};

		VkDebugUtilsMessengerEXT _debugMessenger;
		VkInstance _instance;

		[[nodiscard]] static VkBool32 DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
		[[nodiscard]] static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator);
	};
}