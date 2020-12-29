#include "Windows.h"
#include "platform.h"
#include "game.h"
#include "renderer.h"
#include "obj_loader.h"

template< class ShaderClass >
std::string GetLatestProfile(RendererState* rs);

template<>
std::string GetLatestProfile<ID3D11VertexShader>(RendererState* rs)
{
	assert(rs->device);

	// Query the current feature level:
	D3D_FEATURE_LEVEL featureLevel = rs->device->GetFeatureLevel();

	switch (featureLevel)
	{
	case D3D_FEATURE_LEVEL_11_1:
	case D3D_FEATURE_LEVEL_11_0:
	{
		return "vs_5_0";
	}
	break;
	case D3D_FEATURE_LEVEL_10_1:
	{
		return "vs_4_1";
	}
	break;
	case D3D_FEATURE_LEVEL_10_0:
	{
		return "vs_4_0";
	}
	break;
	case D3D_FEATURE_LEVEL_9_3:
	{
		return "vs_4_0_level_9_3";
	}
	break;
	case D3D_FEATURE_LEVEL_9_2:
	case D3D_FEATURE_LEVEL_9_1:
	{
		return "vs_4_0_level_9_1";
	}
	break;
	}

	return "";
}

template<>
std::string GetLatestProfile<ID3D11PixelShader>(RendererState* rs)
{
	assert(rs->device);

	// Query the current feature level:
	D3D_FEATURE_LEVEL featureLevel = rs->device->GetFeatureLevel();
	switch (featureLevel)
	{
	case D3D_FEATURE_LEVEL_11_1:
	case D3D_FEATURE_LEVEL_11_0:
	{
		return "ps_5_0";
	}
	break;
	case D3D_FEATURE_LEVEL_10_1:
	{
		return "ps_4_1";
	}
	break;
	case D3D_FEATURE_LEVEL_10_0:
	{
		return "ps_4_0";
	}
	break;
	case D3D_FEATURE_LEVEL_9_3:
	{
		return "ps_4_0_level_9_3";
	}
	break;
	case D3D_FEATURE_LEVEL_9_2:
	case D3D_FEATURE_LEVEL_9_1:
	{
		return "ps_4_0_level_9_1";
	}
	break;
	}
	return "";
}

template< class ShaderClass >
ShaderClass* CreateShader(RendererState* rs, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage);

template<>
ID3D11VertexShader* CreateShader<ID3D11VertexShader>(RendererState* rs, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage)
{
	assert(rs->device);
	assert(pShaderBlob);

	ID3D11VertexShader* pVertexShader = nullptr;
	rs->device->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pVertexShader);

	return pVertexShader;
}

template<>
ID3D11PixelShader* CreateShader<ID3D11PixelShader>(RendererState* rs, ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage)
{
	assert(rs->device);
	assert(pShaderBlob);

	ID3D11PixelShader* pPixelShader = nullptr;
	rs->device->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pPixelShader);

	return pPixelShader;
}

template< class ShaderClass >
ShaderClass* LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& _profile)
{
	ID3DBlob* pShaderBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;
	ShaderClass* pShader = nullptr;

	std::string profile = _profile;
	if (profile == "latest")
	{
		profile = GetLatestProfile<ShaderClass>();
	}
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
	flags |= D3DCOMPILE_DEBUG;
#endif

	HRESULT hr = D3DCompileFromFile(fileName.c_str(), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), profile.c_str(),
		flags, 0, &pShaderBlob, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			std::string errorMessage = (char*)pErrorBlob->GetBufferPointer();
			OutputDebugStringA(errorMessage.c_str());

			SafeRelease(pShaderBlob);
			SafeRelease(pErrorBlob);
		}

		return false;
	}

	pShader = CreateShader<ShaderClass>(pShaderBlob, nullptr);

	SafeRelease(pShaderBlob);
	SafeRelease(pErrorBlob);

	return pShader;
}

inline f32 clamp(f32 min, f32 max, f32 val) {
	if (val > max) {
		return max;
	} else if (val < min) {
		return min;
	} else {
		return val;
	}
} 

void addMesh(std::vector<VertexPosColor>* vertices, std::vector<u32>* indices, std::vector<f64>* v, std::vector<u32>* indcs, std::vector<f64>* n) {
	f32 x, y, z;
	for (u32 i = 0; i < v->size() / 3; ++i) {
		x = (*v)[i*3];
		y = (*v)[i*3+1];
		z = (*v)[i*3+2];

		vertices->push_back({
			XMFLOAT3(x, y, z),
			XMFLOAT3(clamp(0.0f, 1.0f, x), clamp(0.0f, 1.0f, y), clamp(0.0f, 1.0f, z)),
			XMFLOAT3((*n)[i*3], (*n)[i*3+1], (*n)[i*3+2])
			});
	}

	for (u32 i = 0; i < indcs->size(); ++i) {
		indices->push_back((*indcs)[i]);
	}
}

struct ModelData {
	std::vector<f64> v;
	std::vector<u32> i;
	std::vector<f64> n;
	std::vector<f32> uv;
};

i32 createVertexBuffer(RendererState* rs, u32 size) {
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = size;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;

	i32 bufferIndex = rs->vertexBuffersCount;
	HRESULT hr = rs->device->CreateBuffer(&bufferDesc, NULL, &rs->vertexBuffers[bufferIndex]);
	if (!SUCCEEDED(hr)) {
		bufferIndex = -1;
	}

	rs->vertexBuffersCount++;

	return bufferIndex;
}

bool LoadContent(RendererState* rs)
{
	assert(rs->device);

	//std::vector<u32> allIndices;
	
	ModelData md1;
	FileMemory objFile = debug_read_entire_file("Data/Models/bny.obj"); // Suzanne
	loadObjModel(&objFile, &md1.v, &md1.i, &md1.n, &md1.uv);
	//addMesh(&v, &allIndices, &md.v, &md.i, &md.n);
	rs->g_indexCount1 = md1.i.size();

	std::vector<VertexPosUV> v1;
	f32 x, y, z;
	for (u32 i = 0; i < md1.v.size() / 3; ++i) {
		x = -md1.v[i*3];
		y = md1.v[i*3+1];
		z = -md1.v[i*3+2];

		v1.push_back({
			XMFLOAT3(x, y, z),
			XMFLOAT3(-md1.n[i*3], md1.n[i*3+1], -md1.n[i*3+2]),
			//XMFLOAT3(clamp(0.0f, 1.0f, x), clamp(0.0f, 1.0f, y), clamp(0.0f, 1.0f, z))
			// XMFLOAT3(0.53333, 0.84705, 0.69019),
			XMFLOAT2(md1.uv[i*2], md1.uv[i*2 + 1])
			});
	}
	rs->g_vertexCount1 = v1.size();

	// ------------------------------------------------
	// Create and initialize the vertex buffer.
	i32 bufferIndex = createVertexBuffer(rs, sizeof(v1[0]) * rs->g_vertexCount1);
	D3D11_SUBRESOURCE_DATA resourceData1;
	ZeroMemory(&resourceData1, sizeof(D3D11_SUBRESOURCE_DATA));
	resourceData1.pSysMem = v1.data();
	// fill the buffer
	rs->deviceCtx->UpdateSubresource(rs->vertexBuffers[bufferIndex], 0, nullptr, resourceData1.pSysMem, 0, 0);

//#ifdef _DEBUG
//	rs->d3dDebug->ReportLiveDeviceObjects( D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL );
//#endif // _DEBUG
	

	// Create and initialize the index buffer.
	D3D11_BUFFER_DESC indexBufferDesc1;
	ZeroMemory(&indexBufferDesc1, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc1.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc1.ByteWidth = sizeof(md1.i[0]) * rs->g_indexCount1;
	indexBufferDesc1.CPUAccessFlags = 0;
	indexBufferDesc1.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA resourceData2;
	ZeroMemory(&resourceData2, sizeof(D3D11_SUBRESOURCE_DATA));
	resourceData2.pSysMem = md1.i.data(); 

	HRESULT hr = rs->device->CreateBuffer(&indexBufferDesc1, &resourceData2, &rs->g_d3dIndexBuffer1);
	if (FAILED(hr))
	{
		return false;
	}

	// ---------------- 2nd object -------------------
	ModelData md2;
	FileMemory objFile2 = debug_read_entire_file("Data/Models/Suzanne.obj");
	loadObjModel(&objFile2, &md2.v, &md2.i, &md2.n, &md2.uv);
	//addMesh(&v, &allIndices, &md.v, &md.i, &md.n);
	rs->g_indexCount2 = md2.i.size();

	std::vector<VertexPosColor> v2;
	for (u32 i = 0; i < md2.v.size() / 3; ++i) {
		x = -md2.v[i*3];
		y = md2.v[i*3+1];
		z = -md2.v[i*3+2];

		v2.push_back({
			XMFLOAT3(x, y, z),
			XMFLOAT3(-md2.n[i*3], md2.n[i*3+1], -md2.n[i*3+2]),
			XMFLOAT3(0.53333, 0.84705, 0.69019)
			//XMFLOAT3(clamp(0.0f, 1.0f, x), clamp(0.0f, 1.0f, y), clamp(0.0f, 1.0f, z))
			});
	}
	rs->g_vertexCount2 = v2.size();

	bufferIndex = createVertexBuffer(rs, sizeof(v2[0]) * rs->g_vertexCount2);

	D3D11_SUBRESOURCE_DATA resourceData3;
	ZeroMemory(&resourceData3, sizeof(D3D11_SUBRESOURCE_DATA));
	resourceData3.pSysMem = v2.data();
	rs->deviceCtx->UpdateSubresource(rs->vertexBuffers[bufferIndex], 0, nullptr, resourceData3.pSysMem, 0, 0);

	D3D11_BUFFER_DESC indexBufferDesc2;
	ZeroMemory(&indexBufferDesc2, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc2.ByteWidth = sizeof(md2.i[0]) * rs->g_indexCount2;
	indexBufferDesc2.CPUAccessFlags = 0;
	indexBufferDesc2.Usage = D3D11_USAGE_DEFAULT; 

	D3D11_SUBRESOURCE_DATA resourceData4;
	ZeroMemory(&resourceData4, sizeof(D3D11_SUBRESOURCE_DATA));
	resourceData4.pSysMem = md2.i.data();
	
	hr = rs->device->CreateBuffer(&indexBufferDesc2, &resourceData4, &rs->g_d3dIndexBuffer2);
	if (FAILED(hr))
	{
		return false;
	}
	// ------------------------------------------------

	// Create the constant buffers for the variables defined in the vertex shader.
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));

	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(XMMATRIX);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	hr = rs->device->CreateBuffer(&constantBufferDesc, nullptr, &rs->g_d3dConstantBuffers[CB_Appliation]);
	if (FAILED(hr))
	{
		return false;
	}
	hr = rs->device->CreateBuffer(&constantBufferDesc, nullptr, &rs->g_d3dConstantBuffers[CB_Object]);
	if (FAILED(hr))
	{
		return false;
	}
	constantBufferDesc.ByteWidth = sizeof(PerFrameBufferData);
	hr = rs->device->CreateBuffer(&constantBufferDesc, nullptr, &rs->g_d3dConstantBuffers[CB_Frame]);
	if (FAILED(hr))
	{
		return false;
	}

	// vertex shader
	hr = rs->device->CreateVertexShader(g_SimpleVertexShader, sizeof(g_SimpleVertexShader), nullptr, &rs->g_d3dSimpleVertexShader);
	if (FAILED(hr))
	{
		return false;
	}

	// Create the input layout for the vertex shader.
	D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor, Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor, Normal),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor, Color),    D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	hr = rs->device->CreateInputLayout(
		vertexLayoutDesc,
		ArrayCount(vertexLayoutDesc),
		g_SimpleVertexShader,
		ArrayCount(g_SimpleVertexShader),
		&rs->simpleShaderInputLayout);

	if (FAILED(hr))
	{
		return false;
	}

	// vertex shader2
	hr = rs->device->CreateVertexShader(g_TexVertexShader, sizeof(g_TexVertexShader), nullptr, &rs->g_d3dTexVertexShader);
	if (FAILED(hr))
	{
		return false;
	}
	// Create the input layout2
	D3D11_INPUT_ELEMENT_DESC texVertexLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosUV, Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosUV, Normal),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV",       0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(VertexPosUV, UV),       D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (FAILED(rs->device->CreateInputLayout(
		texVertexLayoutDesc,
		ArrayCount(texVertexLayoutDesc),
		g_TexVertexShader,
		ArrayCount(g_TexVertexShader),
		&rs->texShaderInputLayout))) {
		return false;
	}

	// Load the compiled pixel shader.
	hr = rs->device->CreatePixelShader(g_SimplePixelShader, sizeof(g_SimplePixelShader), nullptr, &rs->g_d3dSimplePixelShader);
	if (FAILED(hr))
	{
		return false;
	}

	hr = rs->device->CreatePixelShader(g_TexPixelShader, sizeof(g_TexPixelShader), nullptr, &rs->g_d3dTexPixelShader);
	if (FAILED(hr))
	{
		return false;
	}

	// create Texture
	hr = CreateWICTextureFromFile(*rs->device.GetAddressOf(),
		L"Data/Textures/matcap.jpg", //tex.png
		&rs->textureResource, &rs->textureView);
	if (FAILED(hr))
	{
		return false;
	}

	// Setup the projection matrix.
	RECT clientRect;
	GetClientRect(rs->g_WindowHandle, &clientRect);

	// Compute the exact client dimensions.
	// This is required for a correct projection matrix.
	float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
	float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

	rs->g_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), clientWidth / clientHeight, 0.1f, 100.0f);

	rs->deviceCtx->UpdateSubresource(rs->g_d3dConstantBuffers[CB_Appliation], 0, nullptr, &rs->g_ProjectionMatrix, 0, 0);

	return true;
}

GameControllerInput *GetController(GameInput *input, int unsigned controllerIndex)
{
	Assert(controllerIndex < ArrayCount(input->controllers));

	GameControllerInput *result = &input->controllers[controllerIndex];
	return(result);
}

static bool running = true;
static WINDOWPLACEMENT GlobalWindowPosition = { sizeof(GlobalWindowPosition) };

void ToggleFullscreen(HWND Window)
{
	// This follows Raymond Chen's prescription
	// for fullscreen toggling, see:
	// http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx

	DWORD Style = GetWindowLong(Window, GWL_STYLE);
	if (Style & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
		if (GetWindowPlacement(Window, &GlobalWindowPosition) &&
			GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
		{
			SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(Window, HWND_TOP,
				MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
				MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
				MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(Window, &GlobalWindowPosition);
		SetWindowPos(Window, 0, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

void Win32ProcessKeyboardMessage(GameButtonState *newState, bool isDown)
{
	//if (newState->endedDown != isDown)
	//{
	//	NewState->EndedDown = isDown;
	//	++NewState->halfTransitionCount;
	//}
}

void Win32ProcessPendingMessages(GameControllerInput *keyboardController)
{
	MSG message;
	
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		switch (message.message)
		{
			case WM_QUIT:
			{
				running = false;
			} break;

			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				u32 VKCode = (u32)message.wParam;

				bool32 wasDown = ((message.lParam & (1 << 30)) != 0);
				bool32 isDown = ((message.lParam & (1 << 31)) == 0);
				if (wasDown != isDown)
				{
					if (VKCode == 'W')
					{
						Win32ProcessKeyboardMessage(&keyboardController->up, isDown);
					}
					else if (VKCode == 'A')
					{
						Win32ProcessKeyboardMessage(&keyboardController->left, isDown);
					}
					else if (VKCode == 'S')
					{
						Win32ProcessKeyboardMessage(&keyboardController->down, isDown);
					}
					else if (VKCode == 'D')
					{
						Win32ProcessKeyboardMessage(&keyboardController->right, isDown);
					}
					else if (VKCode == 'Q')
					{
						Win32ProcessKeyboardMessage(&keyboardController->leftShoulder, isDown);
					}
					else if (VKCode == 'E')
					{
						Win32ProcessKeyboardMessage(&keyboardController->rightShoulder, isDown);
					}
					else if (VKCode == VK_UP)
					{
						Win32ProcessKeyboardMessage(&keyboardController->actionUp, isDown);
					}
					else if (VKCode == VK_LEFT)
					{
						Win32ProcessKeyboardMessage(&keyboardController->actionLeft, isDown);
					}
					else if (VKCode == VK_DOWN)
					{
						Win32ProcessKeyboardMessage(&keyboardController->actionDown, isDown);
					}
					else if (VKCode == VK_RIGHT)
					{
						Win32ProcessKeyboardMessage(&keyboardController->actionRight, isDown);
					}
					else if (VKCode == VK_ESCAPE)
					{
						Win32ProcessKeyboardMessage(&keyboardController->back, isDown);
					}
					else if (VKCode == VK_SPACE)
					{
						Win32ProcessKeyboardMessage(&keyboardController->start, isDown);
					}

					if (isDown)
					{
						bool32 altKeyWasDown = (message.lParam & (1 << 29));
						if ((VKCode == VK_F4) && altKeyWasDown)
						{
							running = false;
						}
						if ((VKCode == VK_RETURN) && altKeyWasDown)
						{
							if (message.hwnd)
							{
								ToggleFullscreen(message.hwnd);
							}
						}
					}
				}

			} break;

			default:
			{
				TranslateMessage(&message);
				DispatchMessageA(&message);
			} break;
		}
	}
}

// invoked by DispatchMessageA
LRESULT CALLBACK WndProc(HWND Window, UINT Msg,	WPARAM WParam, LPARAM LParam) {

	PAINTSTRUCT paintStruct;
	HDC hDC;

	switch (Msg)
	{
		case WM_CLOSE:
		{
			// TODO(casey): Handle this with a message to the user?
			running = false;
		} break;

		case WM_SETCURSOR:
		{
			//if (DEBUGGlobalShowCursor)
			//{
			//	Result = DefWindowProcA(Window, Message, WParam, LParam);
			//}
			//else
			//{
			//	SetCursor(0);
			//}
		} break;

		case WM_DESTROY:
		{
			running = false; // TODO
			PostQuitMessage(0);
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			// input comes through a non-dispatch message
		} break;

		case WM_PAINT:
		{
			hDC = BeginPaint(Window, &paintStruct);
			EndPaint(Window, &paintStruct);
		}
		break;

		//case WM_PAINT:
		//{
			//PAINTSTRUCT Paint;
			//HDC DeviceContext = BeginPaint(Window, &Paint);
			//win32_window_dimension Dimension = Win32GetWindowDimension(Window);
			//Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
			//	Dimension.Width, Dimension.Height);
			//EndPaint(Window, &Paint);
		//} break;

		default:
		{
			return DefWindowProcA(Window, Msg, WParam, LParam);
		} break;
	}

	return 0;
}

static u32 GlobalWindowWidth = 1920;
static u32 GlobalWindowHeight = 1080;

HWND InitializeWinMain(WNDCLASSEX* windowClass, HINSTANCE hInstance) {//HINSTANCE hInstance, std::string title, std::string wndClassName, int width, int height) {
	windowClass->cbSize = sizeof(WNDCLASSEX);
	windowClass->style = CS_HREDRAW | CS_VREDRAW;
	windowClass->lpfnWndProc = (WNDPROC)WndProc;
	windowClass->cbClsExtra = 0;
	windowClass->cbWndExtra = 0;
	windowClass->hInstance = hInstance;
		// WindowClass.hIcon = LoadIcon((HINSTANCE)NULL, IDI_APPLICATION);
	windowClass->hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass->hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // GetStockObject(WHITE_BRUSH);
	windowClass->lpszMenuName = nullptr;
	windowClass->lpszClassName = "MainWndClass";

	if (!RegisterClassEx(windowClass))
		return FALSE;

	RECT windowRect = { 0, 0, GlobalWindowWidth, GlobalWindowHeight };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	HWND wndHandle = CreateWindowExA(
		0, // WS_EX_TOPMOST | WS_EX_LAYERED,
		windowClass->lpszClassName,
		"Nickel",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		0,
		0,
		hInstance,
		0
	);

	if (!wndHandle) {
		return FALSE;
	}

	return wndHandle;
}

int WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
) {
	if (!XMVerifyCPUSupport())
	{
		MessageBox(nullptr, TEXT("Failed to verify DirectX Math library support."), TEXT("Error"), MB_OK);
		return -1;
	}

	//if (!InitializeWinMain(hInstance, "Title", "MyWndClassName", 800, 600))
	//	return -1;

	Win32State win32State = {};
	WNDCLASSEX WindowClass = {};

	auto wndHandle = InitializeWinMain(&WindowClass, hInstance);

	// display window
	ShowWindow(wndHandle, nShowCmd);
	UpdateWindow(wndHandle);

	RendererState rs;
	rs.g_WindowHandle = wndHandle;
		
	const D3D_FEATURE_LEVEL FEATURE_LEVELS[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Buffer Desc
	/*
	DXGI_MODE_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = GlobalWindowWidth;
	bufferDesc.Height = GlobalWindowHeight;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	*/

	RECT clientRect;
	GetClientRect(rs.g_WindowHandle, &clientRect);
	unsigned int clientWidth = clientRect.right - clientRect.left;
	unsigned int clientHeight = clientRect.bottom - clientRect.top;

	//Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	/*
	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = g_WindowHandle;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	*/

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = clientWidth;
	swapChainDesc.BufferDesc.Height = clientHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate = QueryRefreshRate(clientWidth, clientHeight, false); // TODO vsync
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = rs.g_WindowHandle;
	swapChainDesc.SampleDesc.Count = 8;
	swapChainDesc.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed = TRUE;

	UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL SelectedFeatureLevel;
	HRESULT result = D3D11CreateDeviceAndSwapChain(
		NULL, // defauld adapter
		D3D_DRIVER_TYPE_HARDWARE,
		NULL, // dll handle for software rasterize
		deviceFlags, // D3D11_CREATE_DEVICE_SINGLETHREADED, D3D11_CREATE_DEVICE_DEBUGGABLE, D3D11_CREATE_DEVICE_DEBUG
		FEATURE_LEVELS,
		ArrayCount(FEATURE_LEVELS),
		D3D11_SDK_VERSION,
		&swapChainDesc,
		rs.swapChain.GetAddressOf(),
		rs.device.GetAddressOf(),
		&SelectedFeatureLevel,
		rs.deviceCtx.GetAddressOf());

	if (result == E_INVALIDARG) { // if 11.1 failed, use 11.0
		result = D3D11CreateDeviceAndSwapChain(
			NULL, // defauld adapter
			D3D_DRIVER_TYPE_HARDWARE,
			NULL, // dll handle for software rasterize
			deviceFlags, // D3D11_CREATE_DEVICE_SINGLETHREADED, D3D11_CREATE_DEVICE_DEBUGGABLE, DEBUG
			&FEATURE_LEVELS[1],
			ArrayCount(FEATURE_LEVELS)-1,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			rs.swapChain.GetAddressOf(),
			rs.device.GetAddressOf(),
			&SelectedFeatureLevel,
			rs.deviceCtx.GetAddressOf());
	}

	if (FAILED(result)) {
		return -1;
	}

#ifdef _DEBUG
	// init debug layer
	if( SUCCEEDED( rs.device->QueryInterface( __uuidof(ID3D11Debug), (void**)&rs.d3dDebug ) ) )
	{
		ID3D11InfoQueue *d3dInfoQueue = nullptr;
		if( SUCCEEDED( rs.d3dDebug->QueryInterface( __uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue ) ) )
		{

			d3dInfoQueue->SetBreakOnSeverity( D3D11_MESSAGE_SEVERITY_CORRUPTION, true );
			d3dInfoQueue->SetBreakOnSeverity( D3D11_MESSAGE_SEVERITY_ERROR, true );

			D3D11_MESSAGE_ID hide [] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				// Add more message IDs here as needed
			};

			D3D11_INFO_QUEUE_FILTER filter;
			memset( &filter, 0, sizeof(filter) );
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries( &filter );
			// d3dInfoQueue->Release();
		}

		// d3dDebug->Release();
	}
#endif

	// swap chain stuff
	ID3D11Texture2D* backBuffer;
	result = rs.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	if (FAILED(result)) {
		return -1;
	}

	result = rs.device->CreateRenderTargetView(backBuffer, nullptr, &rs.g_d3dRenderTargetView);
	if (FAILED(result)) {
		return -1;
	}

	SafeRelease(backBuffer);

	// depth/stencil buffer
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
	ZeroMemory(&depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.CPUAccessFlags = 0; // No CPU access required.
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilBufferDesc.Width = GlobalWindowWidth;
	depthStencilBufferDesc.Height = GlobalWindowHeight;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.SampleDesc.Count = 8;
	depthStencilBufferDesc.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	result = rs.device->CreateTexture2D(&depthStencilBufferDesc, nullptr, &rs.g_d3dDepthStencilBuffer);
	if (FAILED(result)) {
		return -1;
	}

	result = rs.device->CreateDepthStencilView(rs.g_d3dDepthStencilBuffer, nullptr, &rs.g_d3dDepthStencilView);
	if (FAILED(result)) {
		return -1;
	}

	// Setup depth/stencil state.
	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthStencilStateDesc.DepthEnable = TRUE;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilStateDesc.StencilEnable = FALSE;

	result = rs.device->CreateDepthStencilState(&depthStencilStateDesc, &rs.g_d3dDepthStencilState);
	if (FAILED(result)) {
		return -1;
	}

	// Setup rasterizer state.
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state object.
	result = rs.device->CreateRasterizerState(&rasterizerDesc, &rs.g_d3dRasterizerState);
	if (FAILED(result)) {
		return -1;
	}

	// Initialize the viewport to occupy the entire client area.
	rs.g_Viewport.Width = static_cast<float>(GlobalWindowWidth);
	rs.g_Viewport.Height = static_cast<float>(GlobalWindowHeight);
	rs.g_Viewport.TopLeftX = 0.0f;
	rs.g_Viewport.TopLeftY = 0.0f;
	rs.g_Viewport.MinDepth = 0.0f;
	rs.g_Viewport.MaxDepth = 1.0f;

	if (!LoadContent(&rs)) {
		return -1;
	}

	GameMemory gameMemory = {};
	gameMemory.permanentStorageSize = Megabytes(256);
	gameMemory.temporaryStorageSize = Gigabytes(1);

	LPVOID baseAddress = 0;
	win32State.TotalSize = gameMemory.permanentStorageSize + gameMemory.temporaryStorageSize;
	win32State.GameMemoryBlock = VirtualAlloc(baseAddress, (size_t)win32State.TotalSize,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE);
	gameMemory.permanentStorage = win32State.GameMemoryBlock;
	gameMemory.temporaryStorage = ((u8 *)gameMemory.permanentStorage +
		gameMemory.permanentStorageSize);

	if (!gameMemory.permanentStorage) { // TODO
	//	return FALSE;
	}

	GameInput input[2] = {};
	GameInput *newInput = &input[0];
	GameInput *oldInput = &input[1];

	if (!gameMemory.isInitialized) {
		Initialize(&gameMemory);
		gameMemory.isInitialized = true;
	}

	while (running) {

		GameControllerInput *oldKeyboardController = GetController(oldInput, 0);
		GameControllerInput *newKeyboardController = GetController(newInput, 0);
		*newKeyboardController = {};
		newKeyboardController->isConnected = true;

		for (int ButtonIndex = 0;
			ButtonIndex < ArrayCount(newKeyboardController->buttons);
			++ButtonIndex)
		{
			newKeyboardController->buttons[ButtonIndex].endedDown =
				oldKeyboardController->buttons[ButtonIndex].endedDown;
		}

		Win32ProcessPendingMessages(newKeyboardController);

		UpdateAndRender(&gameMemory, &rs, newInput);

		GameInput *temp = newInput;
		newInput = oldInput;
		oldInput = temp;
	}
}