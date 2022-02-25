#pragma once

#include "platform.h"
#include "Renderer/renderer.h"
#include "Math.h"
#include "ObjLoader.h"
#include "ResourceManager.h"
#include <vector>

#include "Background.h"

struct GameState {
	RendererState* rs;
};

struct VertexPos {
	XMFLOAT3 Position;
};

struct VertexPosColor {
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT3 Color;
};

struct VertexPosUV {
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 UV;
};

static Nickel::Background background;

namespace Nickel {
	auto NewInitialize(GameMemory* memory) -> void;
	auto Initialize(GameMemory* memory, RendererState* rs) -> void;
	auto NewUpdateAndRender(GameMemory* memory, RendererState* rs, GameInput* input) -> void;
	auto UpdateAndRender(GameMemory* memory, RendererState* rs, GameInput* input) -> void;
	auto SetDefaultPass(const DX11Layer::CmdQueue& cmd, ID3D11RenderTargetView* const* renderTargetView, ID3D11DepthStencilView& depthStencilView) -> void;
	auto LoadObjMeshData(MeshData& modelData, const std::string& path) -> void;
	auto LoadContent(RendererState* rs) -> bool;
}