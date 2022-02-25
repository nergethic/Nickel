#pragma once
#include "DirectX11Includes.h"

namespace Nickel::Renderer::DX11Layer::Core {
	auto Init() -> bool;
	auto Shutdown() -> void;

	auto GetDevice() -> ID3D11Device1* const;
	auto GetCmd() -> ID3D11DeviceContext1* const;

	auto CreateSurface(Platform::Window window) -> Surface;
	auto RemoveSurface(SurfaceId id) -> void;
	auto ResizeSurface(SurfaceId id, u32 width, u32 height) -> void;
	auto GetSurfaceWidth(SurfaceId id) -> u32;
	auto GetSurfaceHeight(SurfaceId id) -> u32;
	auto RenderSurface(SurfaceId id) -> void;
}