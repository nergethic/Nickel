#pragma once

#pragma warning(push, 0) // ignores warnings from external headers
// Link library dependencies
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib") // TODO: check why this doesn't have stuff from dxgi1_3.h (CreateDXGIFactory2)
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")
#pragma warning(pop)

// DirectX includes
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <DirectXColors.h>
#include <WICTextureLoader.h>
#include <dxgi.h>
#include <dxgi1_6.h>

#pragma error(push, 0)
#include "../include/d3d12.h"
#include "../include/d3dx12.h"
#pragma error(pop)

// WRL
#include <wrl/client.h>
#include "../Platform.h"

#include <DirectXTex.h>

#if defined(_DEBUG)
	#define ASSERT_ERROR_RESULT(res) { HRESULT _result = res; if (FAILED(_result)) Nickel::Renderer::DXLayer::AssertD3DResult(_result, GetSourceLocation(std::source_location::current())); }
	#define LOG_ERROR_RESULT(res)    { HRESULT _result = res; if (FAILED(_result)) Nickel::Renderer::DXLayer::LogD3DResult(_result, GetSourceLocation(std::source_location::current())); }
#else
	#define ASSERT_ERROR_RESULT(res) {}
	#define LOG_ERROR_RESULT(res) {}
#endif

// Safely release a COM object
template<typename T>
inline auto SafeRelease(T& ptr) -> void {
	if (ptr != nullptr) {
		ptr->Release();
		ptr = nullptr;
	}
};

struct DxDeleter {
	void operator() (IUnknown* ptr) {
		SafeRelease(ptr);
	}
};

namespace Nickel::Renderer::DXLayer {
	inline auto GetHResultString(HRESULT errCode) -> std::string {
		switch (errCode) {
			// Windows
			case S_OK:           return "S_OK";
			case E_NOTIMPL:      return "E_NOTIMPL";
			case E_NOINTERFACE:  return "E_NOINTERFACE";
			case E_POINTER:      return "E_POINTER";
			case E_ABORT:        return "E_ABORT";
			case E_FAIL:         return "E_FAIL";
			case E_UNEXPECTED:   return "E_UNEXPECTED";
			case E_ACCESSDENIED: return "E_ACCESSDENIED";
			case E_HANDLE:       return "E_HANDLE";
			case E_OUTOFMEMORY:  return "E_OUTOFMEMORY";
			case E_INVALIDARG:   return "E_INVALIDARG";

			// DX11
			case D3D11_ERROR_FILE_NOT_FOUND:                return "D3D11_ERROR_FILE_NOT_FOUND";
			case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS: return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";

			// DXGI
			case DXGI_ERROR_ACCESS_DENIED:                return "DXGI_ERROR_ACCESS_DENIED";
			case DXGI_ERROR_ACCESS_LOST:                  return "DXGI_ERROR_ACCESS_LOST";
			case DXGI_ERROR_ALREADY_EXISTS:               return "DXGI_ERROR_ALREADY_EXISTS";
			case DXGI_ERROR_CANNOT_PROTECT_CONTENT:       return "DXGI_ERROR_CANNOT_PROTECT_CONTENT";
			case DXGI_ERROR_DEVICE_HUNG:                  return "DXGI_ERROR_DEVICE_HUNG";
			case DXGI_ERROR_DEVICE_REMOVED:               return "DXGI_ERROR_DEVICE_REMOVED";
			case DXGI_ERROR_DEVICE_RESET:                 return "DXGI_ERROR_DEVICE_RESET";
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:        return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
			case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:    return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";
			case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";
			case DXGI_ERROR_INVALID_CALL:                 return "DXGI_ERROR_INVALID_CALL";
			case DXGI_ERROR_MORE_DATA:                    return "DXGI_ERROR_MORE_DATA";
			case DXGI_ERROR_NAME_ALREADY_EXISTS:          return "DXGI_ERROR_NAME_ALREADY_EXISTS";
			case DXGI_ERROR_NONEXCLUSIVE:                 return "DXGI_ERROR_NONEXCLUSIVE";
			case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:      return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";
			case DXGI_ERROR_NOT_FOUND:                    return "DXGI_ERROR_NOT_FOUND";
			case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:   return "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED";
			case DXGI_ERROR_REMOTE_OUTOFMEMORY:           return "DXGI_ERROR_REMOTE_OUTOFMEMORY";
			case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:     return "DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE";
			case DXGI_ERROR_SDK_COMPONENT_MISSING:        return "DXGI_ERROR_SDK_COMPONENT_MISSING";
			case DXGI_ERROR_SESSION_DISCONNECTED:         return "DXGI_ERROR_SESSION_DISCONNECTED";
			case DXGI_ERROR_UNSUPPORTED:                  return "DXGI_ERROR_UNSUPPORTED";
			case DXGI_ERROR_WAIT_TIMEOUT:                 return "DXGI_ERROR_WAIT_TIMEOUT";
			case DXGI_ERROR_WAS_STILL_DRAWING:            return "DXGI_ERROR_WAS_STILL_DRAWING";

			default: return "Unhandled HRESULT code: " + std::to_string(static_cast<u32>(errCode));
		}

		return "";
	}

	inline auto GetHResultErrorMessage(HRESULT errCode) -> std::string {
		std::string result = GetHResultString(errCode);
		// TODO: add additional checks for robustness
		// if DXGI_ERROR_DEVICE_REMOVED, DXGI_ERROR_DEVICE_RESET or DXGI_ERROR_DRIVER_INTERNAL_ERROR 
		// ID3D11Device device - GetDeviceRemovedReason();

		return result;
	}

	inline auto AssertD3DResult(HRESULT result, const std::string& locationInfo) -> void {
		const std::string& errorString = GetHResultErrorMessage(result);
		const std::string logStr = errorString + ", " + locationInfo;
		Nickel::Logger::Critical(logStr);
		Assert(FAILED(result));
	}

	inline auto LogD3DResult(HRESULT result, const std::string& locationInfo) -> void {
		const std::string& errorString = GetHResultErrorMessage(result);
		const std::string logStr = errorString + ", " + locationInfo;
		Nickel::Logger::Critical(logStr);
	}
}