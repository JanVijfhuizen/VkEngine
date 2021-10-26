#include "pch.h"
#include "InstanceFactory.h"
#include "Debugger.h"
#include "VkRenderer.h"
#include "WindowSystem.h"

namespace vi 
{
	InstanceFactory::InstanceFactory(VkRenderer& renderer)
	{
		assert(renderer._debugger.CheckValidationLayerSupport());

		auto appInfo = CreateApplicationInfo(renderer);
		auto extensions = GetExtensions(renderer);

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		auto debugInfo = Debugger::CreateInfo();
		renderer._debugger.EnableValidationLayers(debugInfo, createInfo);

		const auto result = vkCreateInstance(&createInfo, nullptr, &renderer._instance);
		assert(!result);
	}

	void InstanceFactory::Cleanup(VkRenderer& renderer)
	{
		vkDestroyInstance(renderer._instance, nullptr);
	}

	VkApplicationInfo InstanceFactory::CreateApplicationInfo(VkRenderer& renderer)
	{
		const auto& windowInfo = renderer._windowSystem.GetVkInfo();
		const auto& name = windowInfo.name.c_str();
		const auto version = VK_MAKE_VERSION(1, 0, 0);

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = name;
		appInfo.applicationVersion = version;
		appInfo.pEngineName = name;
		appInfo.engineVersion = version;
		appInfo.apiVersion = VK_API_VERSION_1_0;

		return appInfo;
	}

	std::vector<const char*> InstanceFactory::GetExtensions(VkRenderer& renderer)
	{
		std::vector<const char*> extensions;
		renderer._windowSystem.GetRequiredExtensions(extensions);
		if (DEBUG)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		return extensions;
	}
}
