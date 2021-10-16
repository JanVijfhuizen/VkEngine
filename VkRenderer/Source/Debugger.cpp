﻿#include "pch.h"
#include "Debugger.h"

namespace vi
{
	void Debugger::Construct(const VkInstance instance)
	{
		if (!DEBUG)
			return;

		_instance = instance;
		auto info = CreateInfo();

		const auto result = CreateDebugUtilsMessengerEXT(instance, &info, nullptr, &_debugMessenger);
		assert(!result);
	}

	void Debugger::Cleanup()
	{
		if (!DEBUG)
			return;

		DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
	}

	bool Debugger::CheckValidationLayerSupport() const
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layer : _validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
				if (strcmp(layer, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}

			if (!layerFound)
				return false;
		}

		return true;
	}

	void Debugger::EnableValidationLayers(VkDebugUtilsMessengerCreateInfoEXT& debugInfo, VkInstanceCreateInfo& instanceInfo) const
	{
		if (!DEBUG)
		{
			instanceInfo.enabledLayerCount = 0;
			return;
		}

		auto& validationLayers = _validationLayers;
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		instanceInfo.ppEnabledLayerNames = validationLayers.data();
		instanceInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugInfo);
	}

	VkDebugUtilsMessengerCreateInfoEXT Debugger::CreateInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = DebugCallback;

		return info;
	}

	VkBool32 Debugger::DebugCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		const VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	VkResult Debugger::CreateDebugUtilsMessengerEXT(const VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void Debugger::DestroyDebugUtilsMessengerEXT(const VkInstance instance, 
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
	{
		const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
			instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (func)
			func(instance, debugMessenger, pAllocator);
	}
}
