#include "pch.h"
#include "UnlitMaterial3d.h"

UnlitMaterial3d::System::System(const uint32_t size) : ShaderSet<UnlitMaterial3d, Frame>(size)
{
}

void UnlitMaterial3d::System::Cleanup()
{
	ShaderSet<UnlitMaterial3d, Frame>::Cleanup();
}

void UnlitMaterial3d::System::Update()
{
	ShaderSet<UnlitMaterial3d, Frame>::Update();
}

void UnlitMaterial3d::System::ConstructInstanceFrame(Frame& frame, UnlitMaterial3d& material, uint32_t denseId)
{
}

void UnlitMaterial3d::System::CleanupInstanceFrame(Frame& frame, UnlitMaterial3d& material, uint32_t denseId)
{
}
