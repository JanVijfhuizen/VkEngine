#pragma once

namespace ce
{
	class Set
	{
	public:
		virtual ~Set() = default;
		virtual void Erase(uint32_t sparseId) = 0;
	};
}