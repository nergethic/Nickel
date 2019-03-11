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

void UpdateAndRender(GameMemory* memory, RendererState* rs, GameInput* input) {

	if (!memory->isInitialized) {
		//VertexPosColor cube2Vertices[] = g_Vertices;

		memory->isInitialized = true;
	}

	// GameState* gs = (GameState*)memory;

	assert(rs->g_d3dDevice);
	assert(rs->g_d3dDeviceContext);

	Vec3 cameraPos;
	cameraPos.x = 0.0f;
	cameraPos.y = 0.0f;
	cameraPos.z = -10.0f;
	
	XMVECTOR eyePosition = XMVectorSet(cameraPos.x, cameraPos.y, cameraPos.z, 1);
	XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
	XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);


	static float angle = -90.0f;
	float radius = 2.0f;
	angle += 1.2f;
	XMFLOAT3 lightPos = XMFLOAT3(radius*cos(XMConvertToRadians(angle)), 0.0f, radius*sin(XMConvertToRadians(angle)));

	PerFrameBufferData data;
	data.viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
	data.cameraPosition = XMFLOAT3(cameraPos.x, cameraPos.y, cameraPos.z);
	data.lightPosition = lightPos;

	rs->g_d3dDeviceContext->ClearDepthStencilView(rs->g_d3dDepthStencilView, NULL, 0.0, 0);
	rs->g_d3dDeviceContext->UpdateSubresource(rs->g_d3dConstantBuffers[CB_Frame], 0, nullptr, &data, 0, 0);

	XMVECTOR rotationAxis = XMVectorSet(0, 1, 0, 0);

	//g_WorldMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(180.0f));
	rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(1.2f, 1.2f, 1.2f));
	//rs->g_WorldMatrix *= XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle/2.0));
	//rs->g_WorldMatrix = XMMatrixIdentity();// XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(180.0f));
	//g_WorldMatrix *= XMMatrixTranslation(2.0*cos(angle), 0.0, 2.0*sin(angle));
	//g_WorldMatrix = XMMatrixIdentity();

	rs->g_d3dDeviceContext->UpdateSubresource(rs->g_d3dConstantBuffers[CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0);


	// RENDER ---------------------------

	Clear(rs, Colors::CornflowerBlue, 1.0f, 0);

	const UINT vertexStride = sizeof(VertexPosColor);
	const UINT offset = 0;

	rs->g_d3dDeviceContext->IASetVertexBuffers(0, 1, &rs->g_d3dVertexBuffer1, &vertexStride, &offset);
	rs->g_d3dDeviceContext->IASetInputLayout(rs->g_d3dInputLayout);
	rs->g_d3dDeviceContext->IASetIndexBuffer(rs->g_d3dIndexBuffer1, DXGI_FORMAT_R32_UINT, 0);
	rs->g_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	rs->g_d3dDeviceContext->VSSetShader(rs->g_d3dVertexShader, nullptr, 0);
	rs->g_d3dDeviceContext->VSSetConstantBuffers(0, 3, rs->g_d3dConstantBuffers);

	rs->g_d3dDeviceContext->RSSetState(rs->g_d3dRasterizerState);
	rs->g_d3dDeviceContext->RSSetViewports(1, &rs->g_Viewport);

	rs->g_d3dDeviceContext->PSSetShader(rs->g_d3dPixelShader, nullptr, 0);

	rs->g_d3dDeviceContext->OMSetRenderTargets(1, &rs->g_d3dRenderTargetView, rs->g_d3dDepthStencilView);
	rs->g_d3dDeviceContext->OMSetDepthStencilState(rs->g_d3dDepthStencilState, 1);

	rs->g_d3dDeviceContext->DrawIndexed(rs->g_indexCount1, 0, 0);

	
	rs->g_WorldMatrix = XMMatrixMultiply(XMMatrixIdentity(), XMMatrixScaling(0.05f, 0.05f, 0.05f));
	rs->g_WorldMatrix *= XMMatrixTranslation(radius*cos(XMConvertToRadians(angle)), 0.0, radius*sin(XMConvertToRadians(angle)));	
	rs->g_d3dDeviceContext->UpdateSubresource(rs->g_d3dConstantBuffers[CB_Object], 0, nullptr, &rs->g_WorldMatrix, 0, 0);

	rs->g_d3dDeviceContext->IASetVertexBuffers(0, 1, &rs->g_d3dVertexBuffer2, &vertexStride, &offset);
	rs->g_d3dDeviceContext->IASetIndexBuffer(rs->g_d3dIndexBuffer2, DXGI_FORMAT_R32_UINT, 0);
	rs->g_d3dDeviceContext->DrawIndexed(rs->g_indexCount2, 0, 0);

	//g_d3dDeviceContext->Draw(g_vertexCount, 0);
	

	rs->g_d3dSwapChain->Present(1, 0);
}