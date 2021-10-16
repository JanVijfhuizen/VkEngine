#include "pch.h"
#include "InstanceFactory.h"
#include "Debugger.h"
#include "WindowSystem.h"

namespace vi 
{
	InstanceFactory::InstanceFactory(const Info& info)
	{
		assert(info.debugger.CheckValidationLayerSupport());

		const auto& windowInfo = info.windowSystem.GetVkInfo();
		const auto& name = windowInfo.name.c_str();
		const auto version = VK_MAKE_VERSION(1, 0, 0);

		auto appInfo = CreateApplicationInfo(info);
		auto extensions = GetExtensions(info);

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		auto debugInfo = Debugger::CreateInfo();
		info.debugger.EnableValidationLayers(debugInfo, createInfo);

		const auto result = vkCreateInstance(&createInfo, nullptr, &info.instance);
		assert(!result);

	}

	VkApplicationInfo InstanceFactory::CreateApplicationInfo(const Info& info)
	{
		const auto& windowInfo = info.windowSystem.GetVkInfo();
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

	std::vector<const char*> InstanceFactory::GetExtensions(const Info& info)
	{
		std::vector<const char*> extensions;
		info.windowSystem.GetRequiredExtensions(extensions);
		if (DEBUG)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		return extensions;
	}
}
