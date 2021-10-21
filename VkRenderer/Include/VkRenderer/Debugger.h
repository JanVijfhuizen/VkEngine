#pragma once

namespace vi
{
	class Debugger final
	{
	public:
		struct Settings final
		{
			std::vector<const char*> validationLayers =
			{
				"VK_LAYER_KHRONOS_validation"
			};
		};

		void Construct(const Settings& settings, VkInstance instance);
		void Cleanup() const;

		[[nodiscard]] bool CheckValidationLayerSupport() const;
		[[nodiscard]] static VkDebugUtilsMessengerCreateInfoEXT CreateInfo();
		void EnableValidationLayers(VkDebugUtilsMessengerCreateInfoEXT& debugInfo, VkInstanceCreateInfo& instanceInfo) const;

		[[nodiscard]] const std::vector<const char*>& GetValidationLayers() const;

	private:
		Settings _settings;

		VkDebugUtilsMessengerEXT _debugMessenger;
		VkInstance _instance;

		[[nodiscard]] static bool IsLayerPresent(const char* layer, std::vector<VkLayerProperties>& layers);

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