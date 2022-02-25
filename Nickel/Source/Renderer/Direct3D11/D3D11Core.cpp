#include "D3D11Core.h"
#include "D3D11Surface.h"

#include "DX11Layer.h"
#include "../renderer.h"
#include "ForwardPass.h"

#include "../Background.h"

namespace Nickel::Renderer::DX11Layer::Core {
	namespace {
		ID3D11Device1* mainDevice = nullptr;
		ID3D11DeviceContext1* cmdList = nullptr;
		CmdQueue cmdQueue{};
		IDXGIFactory2* dxgiFactory = nullptr;
		// debug
		utl::free_list<D3D11Surface> surfaces;

		//
		ID3D11Buffer* commonConstantBuffers[3];
		Background background;
		std::unique_ptr<Camera> mainCamera;

		const static FLOAT g_CLEAR_COLOR[4] = { 0.13333f, 0.13333f, 0.13333f, 1.0f };

		auto SetPipelineState(const ID3D11DeviceContext1& cmd, D3D11_VIEWPORT* viewport, const PipelineState& pipeline) -> void {
			ID3D11DeviceContext1& cmdQueue = const_cast<ID3D11DeviceContext1&>(cmd);
			cmdQueue.RSSetState(pipeline.rasterizerState);
			cmdQueue.OMSetDepthStencilState(pipeline.depthStencilState, 1);
			// cmdQueue.OMSetBlendState() // TODO
			cmdQueue.RSSetViewports(1, viewport); // TOOD: move to render target setup?
		}

		auto Submit(D3D11_VIEWPORT viewport, const DX11Layer::CmdQueue& cmdQueue, const DescribedMesh& mesh) -> void {
			auto cmd = cmdQueue.queue.Get();
			const auto program = mesh.material.program;
			if (program == nullptr) {
				Logger::Error("Mesh material program is null");
				return;
			}

			const auto& gpuData = mesh.gpuData;
			auto& mat = mesh.material;

			if (mesh.gpuData.indexCount == 0) {
				Logger::Warn("Index count is 0!");
				return;
			}

			mat.program->Bind(cmd);
			cmd->IASetPrimitiveTopology(gpuData.topology);

			const auto indexBuffer = gpuData.indexBuffer.buffer.get();
			const auto vertexBuffer = gpuData.vertexBuffer.buffer.get();

			DX11Layer::SetIndexBuffer(*cmd, indexBuffer);
			DX11Layer::SetVertexBuffer(*cmd, vertexBuffer, gpuData.vertexBuffer.stride, gpuData.vertexBuffer.offset);

			if (mat.textures.size() > 0) {
				cmd->PSSetSamplers(0, 1, &mat.textures[0].samplerState);
				for (int i = 0; i < mat.textures.size(); i++) {
					const auto& tex = mat.textures[i];
					cmd->PSSetShaderResources(i, 1, &tex.srv); // TODO: this just puts every texture in slot 0 - fix it
				}
			}

			cmd->VSSetConstantBuffers(0, ArrayCount(commonConstantBuffers), commonConstantBuffers);
			cmd->PSSetConstantBuffers(0, ArrayCount(commonConstantBuffers), commonConstantBuffers);

			const auto vertexConstantBuffer = mat.vertexConstantBuffer.buffer.Get();
			if (vertexConstantBuffer != nullptr) {
				Assert(mat.vertexConstantBuffer.index < D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT)
					cmd->VSSetConstantBuffers(mat.vertexConstantBuffer.index, 1, &vertexConstantBuffer);
			}

			const auto pixelConstantBuffer = mat.pixelConstantBuffer.buffer.Get();
			if (pixelConstantBuffer != nullptr) {
				Assert(mat.pixelConstantBuffer.index < D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT)
					cmd->PSSetConstantBuffers(mat.pixelConstantBuffer.index, 1, &pixelConstantBuffer);
			}

			SetPipelineState(*cmd, &viewport, mesh.material.pipelineState);
			DX11Layer::DrawIndexed(cmdQueue, mesh.gpuData.indexCount, 0, 0);
		}

		auto DrawModel(Camera& camera, D3D11_VIEWPORT viewport, const Nickel::Renderer::DX11Layer::CmdQueue& cmd, const DescribedMesh& mesh, Vec3 offset = { 0.0, 0.0, 0.0 }) -> void { // TODO: const Material* overrideMat = nullptr
			auto c = cmd.queue.Get();
			Assert(c != nullptr);

			// update:
			const auto t = mesh.transform;
			const auto& pos = t.position + offset;
			auto worldMat = XMMatrixScaling(t.scale.x, t.scale.y, t.scale.z)
				* XMMatrixRotationRollPitchYawFromVector(FXMVECTOR{ t.rotation.x, t.rotation.y, t.rotation.z })
				* XMMatrixTranslation(pos.x, pos.y, pos.z);

			const auto& viewProjectionMatrix = camera.GetViewProjectionMatrix();

			PerObjectBufferData data;
			data.modelMatrix = XMMatrixTranspose(worldMat);
			data.viewProjectionMatrix = XMMatrixTranspose(viewProjectionMatrix);
			data.modelViewProjectionMatrix = XMMatrixTranspose(worldMat * viewProjectionMatrix);

			c->UpdateSubresource1(commonConstantBuffers[(u32)ConstantBufferType::CB_Object], 0, nullptr, &data, 0, 0, 0);

			Submit(viewport, cmd, mesh);
		}
	}

	auto GetPerAppUniform() -> ID3D11Buffer* {
		return commonConstantBuffers[(u32)ConstantBufferType::CB_Appliation];
	}

	auto GetPerFrameUniform() -> ID3D11Buffer* {
		return commonConstantBuffers[(u32)ConstantBufferType::CB_Frame];
	}

	auto GetPerObjectUniform() -> ID3D11Buffer* {
		return commonConstantBuffers[(u32)ConstantBufferType::CB_Object];
	}

	auto Init() -> bool {
		if (mainDevice != nullptr)
			Shutdown();

		auto [device, deviceCtx] = DX11Layer::CreateDevice();

		ID3D11Device1* device1 = nullptr;
		ASSERT_ERROR_RESULT(device->QueryInterface(&device1));
		mainDevice = device1;

		device1->GetImmediateContext1(&cmdList);

		ID3D11Debug* d3dDebug = nullptr;
		if constexpr (_DEBUG) {
			d3dDebug = DX11Layer::EnableDebug(*device1, false);
			ASSERT_ERROR_RESULT(d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_SUMMARY | D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL));
		}

		cmdQueue = CmdQueue{ .queue = cmdList, .debug = d3dDebug };

		UINT factoryFlags = 0;
		if constexpr (_DEBUG) {
			factoryFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
		}
		HRESULT result = CreateDXGIFactory2(factoryFlags, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory));
		if (FAILED(result)) {
			Logger::Error("DXGIFactory2 creation failed");
			Assert(false);
		}

		// TODO: This shouldn't be here
		commonConstantBuffers[(u32)ConstantBufferType::CB_Appliation] = DX11Layer::CreateConstantBuffer(mainDevice, sizeof(PerApplicationData));
		commonConstantBuffers[(u32)ConstantBufferType::CB_Object] = DX11Layer::CreateConstantBuffer(mainDevice, sizeof(PerObjectBufferData));
		commonConstantBuffers[(u32)ConstantBufferType::CB_Frame] = DX11Layer::CreateConstantBuffer(mainDevice, sizeof(PerFrameBufferData));
		mainCamera = std::make_unique<Camera>(45.0f, 1.5f, 0.1f, 100.0f);

		return true;
	}

	auto Shutdown() -> void {

	}

	auto GetDevice() -> ID3D11Device1* const {
		return mainDevice;
	}

	auto GetCmd() -> ID3D11DeviceContext1* const {
		return cmdList;
	}

	auto GetMainCamera() -> Nickel::Camera* {
		return mainCamera.get();
	}

	auto CreateSurface(Platform::Window window) -> Surface {
		background.Create();

		SurfaceId id{ surfaces.add(window) };
		surfaces[id].CreateSwapChain(dxgiFactory, mainDevice);

		const auto clientWidth = window.GetWidth();
		const auto clientHeight = window.GetWidth();

		const f32 aspectRatio = static_cast<f32>(clientWidth) / clientHeight;
		const auto& projectionMatrix = mainCamera.get()->GetProjectionMatrix();
		auto data = PerApplicationData{
			.projectionMatrix = XMMatrixTranspose(projectionMatrix),
			.clientData = XMFLOAT3(clientWidth, clientHeight, aspectRatio)
		};

		GetCmd()->UpdateSubresource1(commonConstantBuffers[(u32)ConstantBufferType::CB_Appliation], 0, nullptr, &data, 0, 0, 0);

		return Surface{ id };
		return Surface{ 0 };
	}

	auto RemoveSurface(SurfaceId id) -> void {
		surfaces.remove(id);
	}

	auto ResizeSurface(SurfaceId id, u32 width, u32 height) -> void {
		surfaces[id].Resize();
	}

	auto GetSurfaceWidth(SurfaceId id) -> u32 {
		return surfaces[id].GetWidth();
	}

	auto GetSurfaceHeight(SurfaceId id) -> u32 {
		return surfaces[id].GetHeight();
	}

	auto RenderSurface(SurfaceId id) -> void {
		//cmd.BeginFrame();
		//ID3D12GraphicsCommandList6* cmdList = cmd.GetCmdList();

		//const u32 frameIndex = GetCurrentFrameIndex();
		//if (deferredReleasesFlag[frameIndex]) {
			//ProcessDeferredReleases(frameIndex);
		//}
		//cmd.EndFrame();

		const D3D11Surface& surface = surfaces[id];
		ID3D11Resource* const currentBackBuffer = surface.GetCurrentBackbuffer();

		auto viewport = surface.GetViewport();
		auto scissorRect = surface.GetScissorRect();
		cmdList->RSSetViewports(1, &viewport);
		cmdList->RSSetScissorRects(1, &scissorRect);

		auto rtv = surface.GetCurrentRtv();
		auto dsv = surface.GetCurrentDsv();

		cmdList->OMSetRenderTargets(1, &rtv, dsv);

		DX11Layer::ClearFlag clearFlag = DX11Layer::ClearFlag::CLEAR_COLOR | DX11Layer::ClearFlag::CLEAR_DEPTH;
		DX11Layer::Clear(cmdQueue, static_cast<u32>(clearFlag), rtv, dsv, g_CLEAR_COLOR, 1.0f, 0);

		DrawModel(*mainCamera.get(), viewport, cmdQueue, background.skyboxMesh);

		surface.Present();
	}
}