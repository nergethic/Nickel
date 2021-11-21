#pragma once

#include "platform.h"
#include "Renderer/renderer.h"
#include "Math.h"
#include "ObjLoader.h"
#include "ResourceManager.h"
#include <vector>

#include "Background.h"

static u32 GLOBAL_WINDOW_WIDTH = 1280;
static u32 GLOBAL_WINDOW_HEIGHT = 720;

struct GameState {
	RendererState* rs;
};

struct VertexPos {
	XMFLOAT3 Position;
};

static D3D11_INPUT_ELEMENT_DESC vertexPosDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPos, Position), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

struct VertexPosColor {
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT3 Color;
};

static D3D11_INPUT_ELEMENT_DESC vertexPosColorLayoutDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor, Position), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor, Normal),   D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor, Color),    D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

struct VertexPosUV {
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 UV;
};

static D3D11_INPUT_ELEMENT_DESC vertexPosUVLayoutDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosUV, Position), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosUV, Normal),   D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "UV",       0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(VertexPosUV, UV),       D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

static struct LineVertexData {
	XMFLOAT3 position;
	XMFLOAT3 previous;
	XMFLOAT3 next;
	XMFLOAT3 direction;
};

static Nickel::Background background;

namespace Nickel {
	auto Initialize(GameMemory* memory, RendererState* rs) -> void;
	auto UpdateAndRender(GameMemory* memory, RendererState* rs, GameInput* input) -> void;
	auto SetDefaultPass(const DXLayer11::CmdQueue& cmd, ID3D11RenderTargetView* const* renderTargetView, ID3D11DepthStencilView& depthStencilView) -> void;
	auto LoadObjMeshData(MeshData& modelData, const std::string& path) -> void;
	auto LoadContent(RendererState* rs) -> bool;
}