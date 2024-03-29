#pragma once
#include "platform.h"
#include "renderer.h"
#include "obj_loader.h"
#include <vector>

static u32 GLOBAL_WINDOW_WIDTH = 1080;
static u32 GLOBAL_WINDOW_HEIGHT = 720;
static UINT MSAA_LEVEL = 4;

struct GameState {
	RendererState* rs;
};

struct VertexPosColor {
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT3 Color;
};

struct ModelData {
	std::vector<f64> v;
	std::vector<u32> i;
	std::vector<f64> n;
	std::vector<f32> uv;
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

void Initialize(GameMemory* memory, RendererState* rs);
void UpdateAndRender(GameMemory* memory, RendererState* rs, GameInput* input);
void SetDefaultPass(ID3D11DeviceContext1* deviceCtx, RendererState* rs);