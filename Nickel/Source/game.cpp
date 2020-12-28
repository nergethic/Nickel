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

struct PipelineState {
	ID3D11RasterizerState* rasterizerState;
	ID3D11DepthStencilState* depthStencilState;
	ID3D11InputLayout* inputLayout;
	ID3D11VertexShader* vertexShader;
	ID3D11Buffer* vertexConstantBuffers;
	short vertexConstantBuffersCount;
	ID3D11PixelShader* pixelShader;
	ID3D11Buffer* pixelConstantBuffers;
	short pixelConstantBuffersCount;
	D3D11_PRIMITIVE_TOPOLOGY topology;
};

void SetPipelineState(ID3D11DeviceContext* deviceCtx, RendererState* rs, PipelineState& pipeline) {
	assert(deviceCtx != nullptr);
	assert(rs != nullptr);

	deviceCtx->OMSetRenderTargets(1, &rs->g_d3dRenderTargetView, rs->g_d3dDepthStencilView);

	deviceCtx->RSSetState(pipeline.rasterizerState);
	deviceCtx->OMSetDepthStencilState(pipeline.depthStencilState, 1);

	// deviceCtx->OMSetBlendState(); // TODO

	deviceCtx->IASetPrimitiveTopology(pipeline.topology);
	deviceCtx->IASetInputLayout(pipeline.inputLayout);
	deviceCtx->VSSetShader(pipeline.vertexShader, nullptr, 0);

	if (pipeline.vertexConstantBuffersCount == 0)
		deviceCtx->VSSetConstantBuffers(0, 4, rs->zeroBuffer);
	else
		deviceCtx->VSSetConstantBuffers(0, pipeline.vertexConstantBuffersCount, (ID3D11Buffer *const*)pipeline.vertexConstantBuffers);

	deviceCtx->PSSetShader(pipeline.pixelShader, nullptr, 0);
	// deviceCtx->PSSetConstantBuffers(); // TODO

	deviceCtx->RSSetViewports(1, &rs->g_Viewport);
}

void UpdateAndRender(GameMemory* memory, RendererState* rs, GameInput* input) {

	if (!memory->isInitialized) {
		//VertexPosColor cube2Vertices[] = g_Vertices;

		memory->isInitialized = true;
	}

	// GameState* gs = (GameState*)memory;

	Vec3 cameraPos;
	cameraPos.x = 0.0f;
	cameraPos.y = 0.0f;
	cameraPos.z = -10.0f;
	
	XMVECTOR eyePosition = XMVectorSet(cameraPos.x, cameraPos.y, cameraPos.z, 1);
	XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
	XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);

	static float angle = -90.0f;
	float radius = 2.1f;
	angle += 1.2f;
	XMFLOAT3 lightPos = XMFLOAT3(radius*cos(XMConvertToRadians(angle)), 0.0f, radius*sin(XMConvertToRadians(angle)));

	PerFrameBufferData data;
	data.viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
	data.cameraPosition = XMFLOAT3(cameraPos.x, cameraPos.y, cameraPos.z);
	data.lightPosition = lightPos;

	assert(rs->device);
	assert(rs->deviceCtx);
	ID3D11DeviceContext* deviceCtx = *rs->deviceCtx.GetAddressOf();

	deviceCtx->ClearDepthStencilView(rs->g_d3dDepthStencilView, NULL, 0.0, 0);
	deviceCtx->UpdateSubresource(rs->g_d3dConstantBuffers[CB_Frame], 0, nullptr, &data, 0, 0);

	XMVECTOR rotationAxis = XMVectorSet(0, 1, 0, 0);

	//g_WorldMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(180.0f));
	rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(12.0f, 12.0f, 12.0f));
	rs->g_WorldMatrix *= XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle/2.0));
	rs->g_WorldMatrix *= XMMatrixTranslation(0.0f, -2.0f, 0.0f);
	//rs->g_WorldMatrix = XMMatrixIdentity();// XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(180.0f));
	//g_WorldMatrix *= XMMatrixTranslation(2.0*cos(angle), 0.0, 2.0*sin(angle));
	//g_WorldMatrix = XMMatrixIdentity();

	deviceCtx->UpdateSubresource(rs->g_d3dConstantBuffers[CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0);


	// RENDER ---------------------------
	const FLOAT clearColor[4] = {0.13333f, 0.13333f, 0.13333f, 1.0f};
	Clear(rs, clearColor, 1.0f, 0);

	// pipeline 1 data:
	PipelineState pip1;
	pip1.depthStencilState = rs->g_d3dDepthStencilState;
	pip1.rasterizerState   = rs->g_d3dRasterizerState;
	pip1.inputLayout       = rs->texShaderInputLayout;
	pip1.vertexShader      = rs->g_d3dTexVertexShader;
	pip1.vertexConstantBuffers = (ID3D11Buffer*)rs->g_d3dConstantBuffers;
	pip1.vertexConstantBuffersCount = ArrayCount(rs->g_d3dConstantBuffers);
	pip1.pixelShader       = rs->g_d3dTexPixelShader;
	pip1.topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	SetPipelineState(deviceCtx, rs, pip1);

	const UINT vertexStride = sizeof(VertexPosColor);
	const UINT texVertexStride = sizeof(VertexPosUV);
	const UINT offset = 0;

	deviceCtx->IASetIndexBuffer(rs->g_d3dIndexBuffer1, DXGI_FORMAT_R32_UINT, 0);
	deviceCtx->IASetVertexBuffers(0, 1, &rs->vertexBuffers[0], &texVertexStride, &offset);
	deviceCtx->PSSetShaderResources(0, 1, &rs->textureView);
	deviceCtx->PSSetSamplers(0, 1, &rs->texSamplerState);
	deviceCtx->DrawIndexed(rs->g_indexCount1, 0, 0);

	// --- 2nd object
	rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(0.5f, 0.5f, 0.5f));
	rs->g_WorldMatrix *= XMMatrixTranslation(radius*cos(XMConvertToRadians(angle)), 0.0, radius*sin(XMConvertToRadians(angle)));

	PipelineState pip2;
	pip2.depthStencilState = rs->g_d3dDepthStencilState;
	pip2.rasterizerState   = rs->g_d3dRasterizerState;
	pip2.inputLayout       = rs->texShaderInputLayout;
	pip2.vertexShader      = rs->g_d3dSimpleVertexShader;
	pip2.vertexConstantBuffers = (ID3D11Buffer*)rs->g_d3dConstantBuffers;
	pip2.vertexConstantBuffersCount = ArrayCount(rs->g_d3dConstantBuffers);
	pip2.pixelShader       = rs->g_d3dSimplePixelShader;
	pip2.topology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	SetPipelineState(deviceCtx, rs, pip2);

	deviceCtx->UpdateSubresource(rs->g_d3dConstantBuffers[CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0);

	deviceCtx->IASetInputLayout(rs->simpleShaderInputLayout);
	deviceCtx->IASetIndexBuffer(rs->g_d3dIndexBuffer2, DXGI_FORMAT_R32_UINT, 0);
	deviceCtx->IASetVertexBuffers(0, 1, &rs->vertexBuffers[1], &vertexStride, &offset);
	
	//deviceCtx->VSSetConstantBuffers(0, 3, rs->g_d3dConstantBuffers);

	//deviceCtx->RSSetState(rs->g_d3dRasterizerState);
	//deviceCtx->RSSetViewports(1, &rs->g_Viewport);

	//deviceCtx->OMSetRenderTargets(1, &rs->g_d3dRenderTargetView, rs->g_d3dDepthStencilView);
	//deviceCtx->OMSetDepthStencilState(rs->g_d3dDepthStencilState, 1);
	deviceCtx->DrawIndexed(rs->g_indexCount2, 0, 0);

	//deviceCtx->Draw(g_vertexCount, 0);

	rs->swapChain->Present(1, 0);
}