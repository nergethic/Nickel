#include "Windows.h"
#include "platform.h"
#include "game.h"
//#include "Renderer/renderer.h"

static bool running = true;
static WINDOWPLACEMENT GlobalWindowPosition = { sizeof(GlobalWindowPosition) };

auto GetController(GameInput *input, u32 controllerIndex) -> GameControllerInput* {
	Assert(controllerIndex < ArrayCount(input->controllers));

	GameControllerInput *result = &input->controllers[controllerIndex];
	return(result);
}

auto ToggleFullscreen(HWND Window) -> void {
	// Raymond Chen's article: http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx

	DWORD Style = GetWindowLong(Window, GWL_STYLE);
	if (Style & WS_OVERLAPPEDWINDOW) {
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
	else {
		SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(Window, &GlobalWindowPosition);
		SetWindowPos(Window, 0, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

auto Win32ProcessKeyboardMessage(GameButtonState *newState, bool isDown) -> void {
	//if (newState->endedDown != isDown)
	//{
	//	NewState->EndedDown = isDown;
	//	++NewState->halfTransitionCount;
	//}
}

auto Win32ProcessPendingMessages(GameControllerInput *keyboardController) -> void {
	MSG message;
	
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
		switch (message.message) {
			case WM_QUIT: {
				running = false;
			} break;

			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP: {
				u32 VKCode = (u32)message.wParam;

				bool32 wasDown = ((message.lParam & (1 << 30)) != 0);
				bool32 isDown = ((message.lParam & (1 << 31)) == 0);
				if (wasDown != isDown) {
					if (VKCode == 'W') {
						Win32ProcessKeyboardMessage(&keyboardController->up, isDown);
					}
					else if (VKCode == 'A') {
						Win32ProcessKeyboardMessage(&keyboardController->left, isDown);
					}
					else if (VKCode == 'S') {
						Win32ProcessKeyboardMessage(&keyboardController->down, isDown);
					}
					else if (VKCode == 'D') {
						Win32ProcessKeyboardMessage(&keyboardController->right, isDown);
					}
					else if (VKCode == 'Q') {
						Win32ProcessKeyboardMessage(&keyboardController->leftShoulder, isDown);
					}
					else if (VKCode == 'E') {
						Win32ProcessKeyboardMessage(&keyboardController->rightShoulder, isDown);
					}
					else if (VKCode == VK_UP) {
						Win32ProcessKeyboardMessage(&keyboardController->actionUp, isDown);
					}
					else if (VKCode == VK_LEFT) {
						Win32ProcessKeyboardMessage(&keyboardController->actionLeft, isDown);
					}
					else if (VKCode == VK_DOWN) {
						Win32ProcessKeyboardMessage(&keyboardController->actionDown, isDown);
					}
					else if (VKCode == VK_RIGHT) {
						Win32ProcessKeyboardMessage(&keyboardController->actionRight, isDown);
					}
					else if (VKCode == VK_ESCAPE) {
						Win32ProcessKeyboardMessage(&keyboardController->back, isDown);
					}
					else if (VKCode == VK_SPACE) {
						Win32ProcessKeyboardMessage(&keyboardController->start, isDown);
					}

					if (isDown) {
						bool32 altKeyWasDown = (message.lParam & (1 << 29));
						if ((VKCode == VK_F4) && altKeyWasDown) {
							running = false;
						}
						if (VKCode == VK_ESCAPE) {
							running = false;
						}

						if ((VKCode == VK_RETURN) && altKeyWasDown) {
							if (message.hwnd) {
								ToggleFullscreen(message.hwnd);
							}
						}
					}
				}

			} break;

			default: {
				TranslateMessage(&message);
				DispatchMessageA(&message);
			} break;
		}
	}
}

// invoked by DispatchMessageA
auto CALLBACK WndProc(HWND Window, UINT Msg,	WPARAM WParam, LPARAM LParam) -> LRESULT {

	PAINTSTRUCT paintStruct;
	HDC hDC;

	switch (Msg) {
		case WM_CLOSE: {
			// TODO: Handle this with a message to the user?
			running = false;
		} break;

		case WM_SETCURSOR: {
			//if (DEBUGGlobalShowCursor)
			//{
			//	Result = DefWindowProcA(Window, Message, WParam, LParam);
			//}
			//else
			//{
			//	SetCursor(0);
			//}
		} break;

		case WM_DESTROY: {
			running = false; // TODO
			PostQuitMessage(0);
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_SYSCHAR:
		case WM_KEYUP: {
			// NOTE: input comes through a non-dispatch message
		} break;

		case WM_PAINT: {
			hDC = BeginPaint(Window, &paintStruct);
			EndPaint(Window, &paintStruct);
		}
		break;

		//case WM_PAINT: {
			//PAINTSTRUCT Paint;
			//HDC DeviceContext = BeginPaint(Window, &Paint);
			//win32_window_dimension Dimension = Win32GetWindowDimension(Window);
			//Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
			//	Dimension.Width, Dimension.Height);
			//EndPaint(Window, &Paint);
		//} break;

		default: {
			return DefWindowProcA(Window, Msg, WParam, LParam);
		} break;
	}

	return 0;
}

auto InitializeWinMain(WNDCLASSEX* windowClass, HINSTANCE hInstance) -> HWND { //HINSTANCE hInstance, std::string title, std::string wndClassName, int width, int height) {
	Assert(windowClass != nullptr);

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

	RECT windowRect = { 0, 0, GLOBAL_WINDOW_WIDTH, GLOBAL_WINDOW_HEIGHT };
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

auto GetClientResolution(HWND wndHandle) -> std::pair<u32, u32> {
	RECT clientRect;
	GetClientRect(wndHandle, &clientRect);

	return {
		clientRect.right  - clientRect.left,
		clientRect.bottom - clientRect.top
	};
}

auto AllocateGameMemory(GameMemory& gameMemory, Win32State& win32State) -> void {
	gameMemory.permanentStorageSize = Megabytes(256);
	gameMemory.temporaryStorageSize = Gigabytes(1);

	LPVOID baseAddress = 0;
	win32State.TotalSize = gameMemory.permanentStorageSize + gameMemory.temporaryStorageSize;
	win32State.GameMemoryBlock = VirtualAlloc(baseAddress, (size_t)win32State.TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	gameMemory.permanentStorage = win32State.GameMemoryBlock;
	gameMemory.temporaryStorage = (reinterpret_cast<u8*>(gameMemory.permanentStorage) + gameMemory.permanentStorageSize);
}

auto InitializeImGui(HWND wndHandle, const RendererState& rs) -> void {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(wndHandle);
	ImGui_ImplDX11_Init(rs.device.Get(), rs.cmdQueue.queue.Get());

	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
	colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
	colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
	colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
	colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_Tab] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
	colors[ImGuiCol_TabActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
	//colors[ImGuiCol_DockingPreview] = ImVec4(1.000f, 0.391f, 0.000f, 0.781f);
	//colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_NavHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

	style->ChildRounding = 4.0f;
	style->FrameBorderSize = 1.0f;
	style->FrameRounding = 2.0f;
	style->GrabMinSize = 7.0f;
	style->PopupRounding = 2.0f;
	style->ScrollbarRounding = 12.0f;
	style->ScrollbarSize = 13.0f;
	style->TabBorderSize = 1.0f;
	style->TabRounding = 0.0f;
	style->WindowRounding = 4.0f;
}

auto UpdateCursorPos(HWND wndHandle, RECT clipRect, u32 clientHeight, u32 clientWidth, GameInput& newInput) -> void {
	ClipCursor(&clipRect);

	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(wndHandle, &mousePos);
	f32 mouseX = static_cast<f32>(mousePos.x);
	f32 mouseY = (f32)((clientHeight - 1) - mousePos.y);

	newInput.mouseX = mouseX;
	newInput.mouseY = mouseY;
	newInput.normalizedMouseX = mouseX / clientWidth;
	if (newInput.normalizedMouseX < 0.0f)
		newInput.normalizedMouseX = 0.0f;
	else if (newInput.normalizedMouseX > 1.0f)
		newInput.normalizedMouseX = 1.0f;

	if (newInput.normalizedMouseY < 0.0f)
		newInput.normalizedMouseY = 0.0f;
	else if (newInput.normalizedMouseY > 1.0f)
		newInput.normalizedMouseY = 1.0f;

	newInput.normalizedMouseY = mouseY / (clientWidth - 1);
}

auto WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) -> int {
	//if (!InitializeWinMain(hInstance, "Title", "MyWndClassName", 800, 600))
	//	return -1;

	WNDCLASSEX WindowClass{};
	auto wndHandle = InitializeWinMain(&WindowClass, hInstance);

	ShowWindow(wndHandle, nShowCmd);
	UpdateWindow(wndHandle);

	auto [clientWidth, clientHeight] = GetClientResolution(wndHandle);
	Assert(clientWidth  == GLOBAL_WINDOW_WIDTH);
	Assert(clientHeight == GLOBAL_WINDOW_HEIGHT);

	RendererState rs = Nickel::Renderer::Initialize(wndHandle, clientWidth, clientHeight);
	//Nickel::Renderer::Init(Nickel::Renderer::GraphicsPlatform::Direct3D11);
	InitializeImGui(wndHandle, rs);

	GameMemory gameMemory{};
	Win32State win32State{};
	AllocateGameMemory(gameMemory, win32State);
	if (!gameMemory.permanentStorage) { // TODO
		// return FALSE;
	}

	GameInput input[2] = {};
	GameInput *newInput = &input[0];
	GameInput *oldInput = &input[1];

	if (!gameMemory.isInitialized) {
		Nickel::Initialize(&gameMemory, &rs);
		gameMemory.isInitialized = true;
	}

	while (running) {
		auto *oldKeyboardController = GetController(oldInput, 0);
		auto *newKeyboardController = GetController(newInput, 0);
		*newKeyboardController = {};
		newKeyboardController->isConnected = true;

		for (int ButtonIndex = 0;
			ButtonIndex < ArrayCount(newKeyboardController->buttons);
			++ButtonIndex)
		{
			newKeyboardController->buttons[ButtonIndex].endedDown =
				oldKeyboardController->buttons[ButtonIndex].endedDown;
		}

		RECT clientRect;
		GetWindowRect(wndHandle, &clientRect);
		UpdateCursorPos(wndHandle, clientRect, clientWidth, clientHeight, *newInput);
		/*
		RECT clientRect;
		GetWindowRect(wndHandle, &clientRect);

		ClipCursor(&clientRect);

		POINT mousePos;
		GetCursorPos(&mousePos);
		ScreenToClient(wndHandle, &mousePos);
		f32 mouseX = static_cast<f32>(mousePos.x);
		f32 mouseY = (f32)((clientHeight-1) - mousePos.y);
		newInput->mouseX = mouseX;
		newInput->mouseY = mouseY;
		newInput->normalizedMouseX = mouseX / clientWidth;
		if (newInput->normalizedMouseX < 0.0f)
			newInput->normalizedMouseX = 0.0f;
		else if (newInput->normalizedMouseX > 1.0f)
			newInput->normalizedMouseX = 1.0f;

		if (newInput->normalizedMouseY < 0.0f)
			newInput->normalizedMouseY = 0.0f;
		else if (newInput->normalizedMouseY > 1.0f)
			newInput->normalizedMouseY = 1.0f;

		newInput->normalizedMouseY = mouseY / (clientWidth-1);
		*/

		Win32ProcessPendingMessages(newKeyboardController);
		Nickel::UpdateAndRender(&gameMemory, &rs, newInput);

		std::swap(newInput, oldInput);
	}

	//ImGui_ImplDX11_Shutdown();
	//ImGui_ImplWin32_Shutdown();
	//ImGui::DestroyContext();
	Nickel::Renderer::Shutdown();
	spdlog::drop_all(); // NOTE: Under VisualStudio, this must be called before main finishes to workaround a known VS issue
}