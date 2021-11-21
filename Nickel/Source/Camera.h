#pragma once

#include "platform.h"
#include "Math.h"
#include "Renderer/Direct3D11/DirectX11Includes.h"

namespace Nickel {
	class Camera {
	public:
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;
		DirectX::XMMATRIX viewProjectionMatrix;

	
		Camera(f32 _fovDegrees, f32 _aspectRatio, f32 _nearClip, f32 _farClip)
			: fov(DirectX::XMConvertToRadians(_fovDegrees)), aspectRatio(_aspectRatio), nearClip(_nearClip), farClip(_farClip)
		{
			Assert(nearClip > 0.0f);
			Assert(farClip > 0.0f);
			Assert(farClip > nearClip);

			position = { 0.0f, 8.0f, -10.0f };
			lookAtPosition = { 0.0f, 0.0f, 0.0f };

			RecalculateMatrices();
		}

		inline auto RecalculateMatrices() -> void {
			viewMatrix = DirectX::XMMatrixLookAtLH(
				DirectX::XMVectorSet(position.x, position.y, position.z, 1.0f),
				DirectX::XMVectorSet(lookAtPosition.x, lookAtPosition.y, lookAtPosition.z, 1.0f),
				DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
			);
			projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearClip, farClip);
			viewProjectionMatrix = viewMatrix * projectionMatrix;
		}

		inline auto GetViewMatrix() -> const DirectX::XMMATRIX& {
			return viewMatrix;
		}

		inline auto GetProjectionMatrix() -> const DirectX::XMMATRIX& {
			return projectionMatrix;
		}

		inline auto GetViewProjectionMatrix() -> const DirectX::XMMATRIX& {
			return viewProjectionMatrix;
		}

		f32 fov;
		f32 aspectRatio;
		f32 nearClip, farClip;

		Vec3 position;
		Vec3 lookAtPosition;
	};
}