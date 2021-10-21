#pragma once

namespace vi
{
	class InstanceFactory final
	{
	public:
		struct Info final
		{
			class WindowSystem& windowSystem;
			const class Debugger& debugger;
			VkInstance& instance;
		};

		explicit InstanceFactory(const Info& info);

	private:
		[[nodiscard]] static VkApplicationInfo CreateApplicationInfo(const Info& info);
		[[nodiscard]] static std::vector<const char*> GetExtensions(const Info& info);
	};
}