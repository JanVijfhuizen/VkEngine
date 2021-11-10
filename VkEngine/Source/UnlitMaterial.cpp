#include "pch.h"
#include "UnlitMaterial.h"

UnlitMaterial::System::System(const uint32_t size) : ShaderSet<UnlitMaterial, Frame>(size)
{
}

void UnlitMaterial::System::Cleanup()
{
	ShaderSet<UnlitMaterial, Frame>::Cleanup();
}

void UnlitMaterial::System::Update()
{
	ShaderSet<UnlitMaterial, Frame>::Update();
}

void UnlitMaterial::System::ConstructInstanceFrame(Frame& frame, UnlitMaterial& material, uint32_t denseId)
{
}

void UnlitMaterial::System::CleanupInstanceFrame(Frame& frame, UnlitMaterial& material, uint32_t denseId)
{
}
