#include "game.h"
#include "renderer.h"

/*
struct renderer_state {
	Device,
	DeviceContext,
	ViewMatrix,
	WorldMatrix,
	ConstantBuffers,
	VertexBuffer,
	IndexBuffer,
	VertexPosColor
};
*/

struct Vec3 {
	f32 x, y, z;
};

/*
void CreateCubeMap() {
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = description.width;
	texDesc.Height = description.height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.CPUAccessFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = texDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	D3D11_SUBRESOURCE_DATA pData[6];
	std::vector<vector4b> d[6]; // 6 images of type vector4b = 4 * unsigned char

	for (int cubeMapFaceIndex = 0; cubeMapFaceIndex < 6; cubeMapFaceIndex++)
	{
		d[cubeMapFaceIndex].resize(description.width * description.height);

		// fill with red color  
		std::fill(
			d[cubeMapFaceIndex].begin(),
			d[cubeMapFaceIndex].end(),
			vector4b(255, 0, 0, 255));

		pData[cubeMapFaceIndex].pSysMem = &d[cubeMapFaceIndex][0];// description.data;
		pData[cubeMapFaceIndex].SysMemPitch = description.width * 4;
		pData[cubeMapFaceIndex].SysMemSlicePitch = 0;
	}

	HRESULT hr = renderer->getDevice()->CreateTexture2D(&texDesc,
		description.data[0] ? &pData[0] : nullptr, &m_pCubeTexture);
	assert(hr == S_OK);

	hr = renderer->getDevice()->CreateShaderResourceView(
		m_pCubeTexture, &SMViewDesc, &m_pShaderResourceView);
	assert(hr == S_OK);
}
*/

const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
const UINT vertexStride = sizeof(VertexPosColor);
const UINT texVertexStride = sizeof(VertexPosUV);
const UINT offset = 0;

void SetDefaultPass(ID3D11DeviceContext1* deviceCtx, RendererState* rs) {
	assert(deviceCtx != nullptr);
	assert(rs != nullptr);
	assert(rs->defaultRenderTargetView != nullptr);
	assert(rs->defaultDepthStencilView != nullptr);

	deviceCtx->OMSetRenderTargets(1, &rs->defaultRenderTargetView, rs->defaultDepthStencilView);
}

void SetPipelineState(ID3D11DeviceContext1* deviceCtx, RendererState* rs, PipelineState& pipeline) {
	assert(deviceCtx != nullptr);
	assert(rs != nullptr);

	deviceCtx->RSSetState(pipeline.rasterizerState);
	deviceCtx->OMSetDepthStencilState(pipeline.depthStencilState, 1);

	// deviceCtx->OMSetBlendState(); // TODO

	deviceCtx->IASetPrimitiveTopology(pipeline.topology);
	deviceCtx->IASetInputLayout(pipeline.inputLayout);
	deviceCtx->VSSetShader(pipeline.vertexShader, nullptr, 0);

	if (pipeline.vertexConstantBuffersCount == 0)
		deviceCtx->VSSetConstantBuffers(0, ArrayCount(rs->zeroBuffer), rs->zeroBuffer);
	else
		deviceCtx->VSSetConstantBuffers(0, pipeline.vertexConstantBuffersCount, (ID3D11Buffer *const*)pipeline.vertexConstantBuffers);

	deviceCtx->PSSetShader(pipeline.pixelShader, nullptr, 0);
	// deviceCtx->PSSetConstantBuffers(); // TODO

	deviceCtx->RSSetViewports(1, &rs->g_Viewport);
}

struct VertexBuffer {
	ID3D11Buffer* buffer;
	D3D11_INPUT_ELEMENT_DESC inputElementDescription;
	UINT stride;
	UINT offset;
};

struct ShaderData {
	const void* pShaderBytecodeWithInputSignature;
	SIZE_T BytecodeLength;
};

void SetVertexBuffer(ID3D11DeviceContext1* deviceCtx, ID3D11Buffer* vertexBuffer, UINT stride, UINT offset) {
	deviceCtx->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
}

void SetIndexBuffer(ID3D11DeviceContext1* deviceCtx, ID3D11Buffer* indexBuffer, DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT, u32 offset = 0) {
	deviceCtx->IASetIndexBuffer(indexBuffer, format, offset);
}

void DrawIndexed(ID3D11DeviceContext1* deviceCtx, UINT indexCount) {
	deviceCtx->DrawIndexed(indexCount, 0, 0);
}

void DrawBunny(ID3D11DeviceContext1* deviceCtx, RendererState* rs, PipelineState pipelineState) {
	SetPipelineState(deviceCtx, rs, pipelineState);

	deviceCtx->PSSetShaderResources(0, 1, &rs->textureView);
	deviceCtx->PSSetSamplers(0, 1, &rs->texSamplerState);

	SetIndexBuffer(deviceCtx, rs->g_d3dIndexBuffer1);
	SetVertexBuffer(deviceCtx, rs->vertexBuffers[0], texVertexStride, offset);

	deviceCtx->UpdateSubresource1(rs->g_d3dConstantBuffers[CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0, 0);

	deviceCtx->DrawIndexed(rs->g_indexCount1, 0, 0);
}

void DrawLight(ID3D11DeviceContext1* deviceCtx, RendererState* rs, PipelineState pipelineState) {
	SetPipelineState(deviceCtx, rs, pipelineState);

	deviceCtx->PSSetShaderResources(0, 1, &rs->textureView);
	deviceCtx->PSSetSamplers(0, 1, &rs->texSamplerState);

	SetIndexBuffer(deviceCtx, rs->g_d3dIndexBuffer1);
	SetVertexBuffer(deviceCtx, rs->vertexBuffers[0], texVertexStride, offset);

	deviceCtx->UpdateSubresource1(rs->g_d3dConstantBuffers[CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0, 0);

	deviceCtx->DrawIndexed(rs->g_indexCount1, 0, 0);
}

void DrawSuzanne(ID3D11DeviceContext1* deviceCtx, RendererState* rs, PipelineState pipelineState) {
	SetPipelineState(deviceCtx, rs, pipelineState);

	deviceCtx->PSSetShaderResources(0, ArrayCount(rs->zeroResourceViews), rs->zeroResourceViews);
	deviceCtx->PSSetSamplers(0, ArrayCount(rs->zeroSamplerStates), rs->zeroSamplerStates);

	SetIndexBuffer(deviceCtx, rs->g_d3dIndexBuffer2);
	SetVertexBuffer(deviceCtx, rs->vertexBuffers[1], vertexStride, offset);

	deviceCtx->UpdateSubresource1(rs->g_d3dConstantBuffers[CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0, 0);

	DrawIndexed(deviceCtx, rs->g_indexCount2);
}

ID3D11InputLayout* CreateInputLayout(ID3D11Device1* device, D3D11_INPUT_ELEMENT_DESC* vertexLayoutDesc, UINT vertexLayoutDescLength, const BYTE* shaderBytecodeWithInputSignature, SIZE_T shaderBytecodeSize) {
	assert(device != nullptr);
	assert(vertexLayoutDesc != nullptr);
	assert(shaderBytecodeWithInputSignature != nullptr);

	ID3D11InputLayout* result = nullptr;

	HRESULT hr = device->CreateInputLayout(
		vertexLayoutDesc,
		vertexLayoutDescLength,
		shaderBytecodeWithInputSignature,
		shaderBytecodeSize,
		&result);

	if (FAILED(hr)) {
		// TODO: log error
		assert(nullptr);
	}

	return result;
}

void ApplyPipeline(ID3D11Device1* device, PipelineState* pipeline) {
	assert(device != nullptr);
	assert(pipeline != nullptr);

	pipeline->inputLayout = CreateInputLayout(device, vertexPosUVLayoutDesc, ArrayCount(vertexPosUVLayoutDesc), g_TexVertexShader, ArrayCount(g_TexVertexShader));
}

void Initialize(GameMemory* memory, RendererState* rs) {
	assert(memory != nullptr);
	assert(rs != nullptr);
	assert(rs->device);
	assert(rs->deviceCtx);

	ID3D11Device1* device = rs->device.Get();

	rs->defaultDepthStencilBuffer = CreateDepthStencilTexture(device, rs->backbufferWidth, rs->backbufferHeight);
	assert(rs->defaultDepthStencilBuffer != nullptr);
	rs->defaultDepthStencilView   = CreateDepthStencilView(device, rs->defaultDepthStencilBuffer);

	auto depthStencilState = CreateDepthStencilState(device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, false);
	auto rasterizerState   = CreateDefaultRasterizerState(device);

	auto pip = &rs->pip;
	pip->depthStencilState          = depthStencilState;
	pip->rasterizerState            = rasterizerState;
	pip->inputLayout                = CreateInputLayout(device, vertexPosUVLayoutDesc, ArrayCount(vertexPosUVLayoutDesc), g_TexVertexShader, ArrayCount(g_TexVertexShader));
	pip->vertexShader               = rs->g_d3dTexVertexShader;
	pip->vertexConstantBuffers      = (ID3D11Buffer*)rs->g_d3dConstantBuffers;
	pip->vertexConstantBuffersCount = ArrayCount(rs->g_d3dConstantBuffers);
	pip->pixelShader                = rs->g_d3dTexPixelShader;
	pip->topology                   = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// ApplyPipeline(device, &pip); // TODO

	auto pip2 = &rs->pip2;
	pip2->depthStencilState          = depthStencilState;
	pip2->rasterizerState            = rasterizerState;
	pip2->inputLayout                = CreateInputLayout(device, vertexPosColorLayoutDesc, ArrayCount(vertexPosColorLayoutDesc), g_SimpleVertexShader, ArrayCount(g_SimpleVertexShader));
	pip2->vertexShader               = rs->g_d3dSimpleVertexShader;
	pip2->vertexConstantBuffers      = (ID3D11Buffer*)rs->g_d3dConstantBuffers;
	pip2->vertexConstantBuffersCount = ArrayCount(rs->g_d3dConstantBuffers);
	pip2->pixelShader                = rs->g_d3dSimplePixelShader;
	pip2->topology                   = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D11_RENDER_TARGET_BLEND_DESC1 blendDescription = { 0 };
	D3D11_BLEND_DESC1 stateDesc = { 0 };
	stateDesc.AlphaToCoverageEnable = true;
	stateDesc.IndependentBlendEnable = true;
	for (int i = 0; i < ArrayCount(stateDesc.RenderTarget); i++) {
		stateDesc.RenderTarget[i] = blendDescription;
	}
	
	// TODO: this is only dummy test code
	ID3D11BlendState1* state;
	device->CreateBlendState1(&stateDesc, &state);
	const FLOAT blendFactor[4] = {0};
	// rs->deviceCtx->OMSetBlendState(state, blendFactor, 0);

	ID3D11DeviceContext1* deviceCtx = rs->deviceCtx.Get();
	deviceCtx->OMSetRenderTargets(1, &rs->defaultRenderTargetView, nullptr);
	deviceCtx->OMSetRenderTargets(1, &rs->defaultRenderTargetView, rs->defaultDepthStencilView);
}

void UpdateAndRender(GameMemory* memory, RendererState* rs, GameInput* input) {
	// GameState* gs = (GameState*)memory;

	Vec3 cameraPos;
	cameraPos.x = 0.0f;
	cameraPos.y = 0.0f;
	cameraPos.z = -10.0f;
	
	XMVECTOR eyePosition = XMVectorSet(cameraPos.x, cameraPos.y, cameraPos.z, 1);
	XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);

	static float angle = -90.0f;
	float radius = 2.1f;
	angle += 1.2f;
	XMFLOAT3 lightPos = XMFLOAT3(radius*cos(XMConvertToRadians(angle)), 0.0f, radius*sin(XMConvertToRadians(angle)));

	PerFrameBufferData frameData;
	frameData.viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
	frameData.cameraPosition = XMFLOAT3(cameraPos.x, cameraPos.y, cameraPos.z);
	frameData.lightPosition = lightPos;

	ID3D11DeviceContext1* deviceCtx = rs->deviceCtx.Get();

	deviceCtx->UpdateSubresource1(rs->g_d3dConstantBuffers[CB_Frame], 0, nullptr, &frameData, 0, 0, 0);

	// RENDER ---------------------------
	const FLOAT clearColor[4] = {0.13333f, 0.13333f, 0.13333f, 1.0f};
	Clear(rs, clearColor, 1.0f, 0);

	SetDefaultPass(deviceCtx, rs);

	rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(12.0f, 12.0f, 12.0f));
	rs->g_WorldMatrix *= XMMatrixTranslation(0.0f, -2.0f, 0.0f);
	DrawBunny(deviceCtx, rs, rs->pip);

	rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(2.0f, 2.0f, 2.0f));
	rs->g_WorldMatrix *= XMMatrixTranslation(lightPos.x, lightPos.y, lightPos.z);
	DrawLight(deviceCtx, rs, rs->pip);
	
	rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(0.5f, 0.5f, 0.5f));
	rs->g_WorldMatrix *= XMMatrixTranslation(radius * cos(XMConvertToRadians(180.0f)), 0.0f, radius * sin(XMConvertToRadians(180.0f)));
	DrawSuzanne(deviceCtx, rs, rs->pip2);

	//deviceCtx->Draw(g_vertexCount, 0);

	rs->swapChain->Present(1, 0);
}