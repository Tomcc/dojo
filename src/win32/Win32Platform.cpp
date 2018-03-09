#include "win32/Win32Platform.h"

#include "Renderer.h"
#include "Game.h"
#include "Table.h"
#include "FontSystem.h"
#include "dojomath.h"
#include "SoundManager.h"
#include "InputSystem.h"
#include "WorkerPool.h"
#include "Path.h"
#include "Keyboard.h"

#include "dojo_win_header.h"
#include "win32/WGL_ARB_multisample.h"
#include "win32/XInputController.h"
#include <glad/glad.h>

#include <FreeImage.h>
#include <GL/wglext.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

using namespace Dojo;

//wglew is a tad outdated
#define CONTEXT_FLAG_NO_ERROR_BIT_KHR    0x00000008

HINSTANCE hInstance; // window app instance
HWND hWindow; // handle for the window
HDC hdc; // handle to device context
HGLRC hglrc; // handle to OpenGL rendering context

#define WINDOWMODE_PROPERTIES (WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS)

static void *getProcAddress(const char *functionName) {
	void *p = (void *)wglGetProcAddress(functionName);
	if (p == 0 or
		(p == (void*)0x1) or (p == (void*)0x2) or (p == (void*)0x3) or
		(p == (void*)-1))
	{
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void *)GetProcAddress(module, functionName);
	}

	return p;
}

void _debugWin32Error(const char* msg, const char* file_source, int line, const char* function) {
	DWORD error = GetLastError();

	gp_assert_handler(
		("Win32 encountered an error: " + utf::to_string((int)error) + " (" + msg + ")").bytes().data(),
		"error != GL_NO_ERROR",
		nullptr,
		line,
		file_source,
		function);
}

#ifndef PUBLISH
#define CHECK_WIN32_ERROR(T, MSG ) { if(not (T) ) { _debugWin32Error( MSG, __FILE__, __LINE__, __FUNCTION__ ); }  }
#else
#define CHECK_WIN32_ERROR(T, MSG ) {}
#endif

Touch::Type win32messageToMouseButton(UINT message) {
	switch (message) {
	case WM_LBUTTONDOWN: //left down
	case WM_LBUTTONUP:
		return Touch::Type::LeftClick;

	case WM_RBUTTONDOWN: //right up
	case WM_RBUTTONUP:
		return Touch::Type::RightClick;

	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		return Touch::Type::LeftClick;

	default:
		return Touch::Type::Tap;
	}
}

LRESULT OnTouch(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	BOOL bHandled = FALSE;

	static std::vector<TOUCHINPUT> inputs;
	inputs.resize(LOWORD(wParam));

	auto& app = (Win32Platform&)Platform::singleton();

	if (GetTouchInputInfo((HTOUCHINPUT)lParam, static_cast<UINT>(inputs.size()), inputs.data(), sizeof(TOUCHINPUT))) {
		for (UINT i = 0; i < inputs.size(); i++) {
			auto& ti = inputs[i];
			POINT ptInput = {
				TOUCH_COORD_TO_PIXEL(ti.x),
				TOUCH_COORD_TO_PIXEL(ti.y)
			};

			ScreenToClient(hWnd, &ptInput);

			//do something with each touch input entry
			if (ti.dwFlags & TOUCHEVENTF_DOWN) {
				app.mousePressed(ptInput.x, ptInput.y, Touch::Type::Tap);
			}
			else if (ti.dwFlags & TOUCHEVENTF_UP) {
				app.mouseReleased(ptInput.x, ptInput.y, Touch::Type::Tap);
			}
			else if (ti.dwFlags & TOUCHEVENTF_MOVE) {
				app.mouseMoved(ptInput.x, ptInput.y);
			}
		}

		bHandled = TRUE;
	}
	else {
		/* handle the error here */
	}

	if (bHandled) {
		// if you handled the message, close the touch input handle and return
		CloseTouchInputHandle((HTOUCHINPUT)lParam);
		return 0;
	}
	else {
		// if you didn't handle the message, let DefWindowProc handle it
		return DefWindowProc(hWnd, WM_TOUCH, wParam, lParam);
	}
}

bool mouseEventIsGesture = false;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	auto& app = (Win32Platform&)Platform::singleton();

	switch (message) {
	case WM_CREATE:
		return 0;
		break;

	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}

	return 0;
	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		//enter / exit unfocused state
	case WM_ACTIVATEAPP:
	case WM_ACTIVATE:
	case WM_SHOWWINDOW:
		if (wparam == false) { //minimized or defocused
			app._fireFocusLost();
		}

		else {
			app._fireFocusGained();
		}

		return 0;

	case WM_MOUSEWHEEL: //mouse wheel moved
		app.mouseWheelMoved((int)((float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA));
		return 0;

	case WM_LBUTTONDOWN: //left down
	case WM_RBUTTONDOWN: //right up
	case WM_MBUTTONDOWN:
		if (not mouseEventIsGesture) {
			app.mousePressed(LOWORD(lparam), HIWORD(lparam), win32messageToMouseButton(message));
		}

		return 0;

	case WM_LBUTTONUP: //left up
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		if (not mouseEventIsGesture) {
			app.mouseReleased(LOWORD(lparam), HIWORD(lparam), win32messageToMouseButton(message));
		}

		mouseEventIsGesture = false;
		return 0;

	case WM_MOUSEMOVE:
		if (not mouseEventIsGesture) {
			app.mouseMoved(LOWORD(lparam), HIWORD(lparam));
		}

		return 0;

	case WM_TOUCH:
		mouseEventIsGesture = true; //ignore the next mouse event
		OnTouch(hwnd, wparam, lparam);
		break;

	case WM_SYSKEYDOWN:
		if (wparam == VK_F4) { //listen for Alt+F4
			PostQuitMessage(1);
			return 0;
		}

		//continues after the jump!!

	case WM_KEYDOWN:

		switch (wparam) {
		case VK_ESCAPE:
#ifdef _DEBUG //close with ESC automagically

			PostQuitMessage(0);
			break;
#endif

		default:

			app.keyPressed(static_cast<int>(wparam));
			break;
		}

		return 0;

	case WM_SYSKEYUP: //needed to catch ALT separately
	case WM_KEYUP:

		app.keyReleased(static_cast<int>(wparam));
		return 0;

	case WM_CHAR:
		return 0;

	case WM_SIZE:

		return 0;

	case WM_ENTERSIZEMOVE: //this message is sent when the window is about to lose control in live resize
		//start a timer to keep getting updates at 30 fps
		//SetTimer( getWin32Window(), 1, 1./30., nullptr );
		//_isInModalLoop = true;

		return 0;

	case WM_TIMER:
		//the window is currently in live resize, manually update the game!
		//if( pGame and pGame->isRunning() )
		//	pGame->update();

		return 0;

	case WM_EXITSIZEMOVE:
		//_isInModalLoop = false;
		//KillTimer( getWin32Window(), 1 );

		return 0;

	case WM_KILLFOCUS:
		//if we were fullscreen in OpenGL, we need to reset the original setup
		/*if( getVideoDriver() and DriverType == ox::video::EDT_OPENGL and getVideoDriver()->isFullscreen() )
		{
			bool success = _changeDisplayMode( false, ox::TDimension() );
			//hide window
			ShowWindow( hWnd, FALSE );

			DEBUG_ASSERT( success );
		}*/
		return 0;

	case WM_SETFOCUS:
		//if we are fullscreen in OpenGL, we need to change the res again
		/*if( getVideoDriver() and DriverType == ox::video::EDT_OPENGL and getVideoDriver()->isFullscreen() )
		{
			bool success = _changeDisplayMode( true, WindowSize );
			ShowWindow( hWnd, TRUE );
			DEBUG_ASSERT( success );
		}*/
		return 0;
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}

//convince NVidia video cards to activate, on notebooks
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

Win32Platform::Win32Platform(const Table& configTable) :
	Platform(configTable),
	dragging(false),
	mMousePressed(false),
	cursorPos(Vector::Zero),
	frameInterval(0),
	mFramesToAdvance(0),
	clientAreaYOffset(0) {
	/*
	#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF |_CRTDBG_LEAK_CHECK_DF );
	#endif
	*/
	//TODO detect locale code
	locale += "en";

	SetProcessDPIAware();

	//detect monitor size
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	_initKeyMap();

}

Win32Platform::~Win32Platform() {

}

void Win32Platform::_adjustWindow() {
	RECT clientSize;
	//GetClientRect(HWnd, &clientSize);
	clientSize.top = 0;
	clientSize.left = 0;
	clientSize.right = windowWidth;
	clientSize.bottom = windowHeight;

	AdjustWindowRect(&clientSize, WINDOWMODE_PROPERTIES, FALSE);

	int realWidth = clientSize.right - clientSize.left;
	int realHeight = clientSize.bottom - clientSize.top;

	int windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
	int windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

	// Make any window in the background repaint themselves
	InvalidateRect(nullptr, nullptr, false);

	SetWindowPos(hWindow, HWND_NOTOPMOST, windowLeft, windowTop, realWidth, realHeight,
		SWP_SHOWWINDOW);

	MoveWindow(hWindow, windowLeft, windowTop, realWidth, realHeight, TRUE);

	//get the new area and see what happened, get the offset
	if (not isFullscreen()) {
		RECT rcClient, rcWind;
		GetClientRect(hWindow, &rcClient);
		GetWindowRect(hWindow, &rcWind);

		clientAreaYOffset = rcWind.top - rcClient.top;
	}

}

bool Win32Platform::_initializeWindow(utf::string_view windowCaption, int w, int h) {
	DEBUG_MESSAGE("Creating " + utf::to_string(w) + "x" + utf::to_string(h) + " window");

	hInstance = (HINSTANCE)GetModuleHandle(nullptr);

	WNDCLASS wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = TEXT("DojoOpenGLWindow");
	wc.lpszMenuName = 0;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	// Register that class with the Windows O/S..
	RegisterClass(&wc);

	RECT rect;
	rect.top = rect.left = 0;
	rect.bottom = rect.top + h + 7;
	rect.right = rect.left + w;

	DWORD dwstyle = WINDOWMODE_PROPERTIES;

	AdjustWindowRect(&rect, dwstyle, true);

	// AdjustWindowRect() expands the RECT
	// so that the CLIENT AREA (drawable region)
	// has EXACTLY the dimensions we specify
	// in the incoming RECT.

	///////////////////
	// NOW we call CreateWindow, using
	// that adjusted RECT structure to
	// specify the width and height of the window.

	hWindow = CreateWindowW(L"DojoOpenGLWindow",
		String::toUTF16(windowCaption).c_str(),
		dwstyle, //non-resizabile
		rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		nullptr, nullptr,
		hInstance, nullptr);

	if (hWindow == nullptr) {
		return false;
	}

	//configure it to receive touches instead of "mouse gestures" where available
	{
		// test for touch
		auto value = GetSystemMetrics(SM_DIGITIZER);

		if (value & NID_MULTI_INPUT or value & NID_INTEGRATED_TOUCH) {
			RegisterTouchWindow(hWindow, TWF_WANTPALM | TWF_FINETOUCH);
		}
	}

	hdc = GetDC(hWindow);
	// CREATE PFD:
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor = { 0 };
	pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
	pixelFormatDescriptor.nVersion = 1;

	pixelFormatDescriptor.dwFlags =
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.dwLayerMask = PFD_MAIN_PLANE;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 16; //allow 565
	pixelFormatDescriptor.cAlphaBits = 0;
	pixelFormatDescriptor.cDepthBits = game->getRequiresDepthBuffer() ? 16 : 0;
	pixelFormatDescriptor.cStencilBits = game->getRequiresDepthBuffer() ? 1 : 0;


	int chosenPixelFormat;
	int MSAALevel = config.getInt("MSAA");
	if (MSAALevel > 0) {
		ChooseAntiAliasingPixelFormat(chosenPixelFormat, config.getInt("MSAA"));
	}
	else {
		chosenPixelFormat = ChoosePixelFormat(hdc, &pixelFormatDescriptor);
	}

	if (chosenPixelFormat == 0) {
		return false;
	}

	if (not SetPixelFormat(hdc, chosenPixelFormat, &pixelFormatDescriptor)) {
		return false;
	}

	hglrc = wglCreateContext(hdc);
	if (not hglrc) {
		FAIL("wglCreateContext");
	}

	// Make the context current
	if (not wglMakeCurrent(hdc, hglrc)) {
		FAIL("Couldn't make the rendering context current");
	}

	//if we can use OpenGL 3.x, do that and initialize with custom context attributes
	if (auto wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB")) {
		//destroy the current context
		if (not wglMakeCurrent(nullptr, nullptr) or not wglDeleteContext(hglrc)) {
			FAIL("Cannot remove dummy context");
		}

		const int glVersion[] = {
			3, 2
		};

		int wglAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, glVersion[0],
			WGL_CONTEXT_MINOR_VERSION_ARB, glVersion[1],
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,// core versus compatibility
			WGL_CONTEXT_FLAGS_ARB, CONTEXT_FLAG_NO_ERROR_BIT_KHR,
			0	// end array value, required
		};

		hglrc = wglCreateContextAttribsARB(hdc, 0, wglAttributes);
		DEBUG_ASSERT(hglrc, "Cannot create context");

		if (not wglMakeCurrent(hdc, hglrc)) {
			FAIL("Cannot use this context");
		}
	}

	auto success = gladLoadGLES2Loader(getProcAddress);
	DEBUG_ASSERT(success, "Cannot load opengl");


	// and show.
	ShowWindow(hWindow, SW_SHOWNORMAL);

	_setFullscreen(mFullscreen);

	return true;
}

void Win32Platform::_setFullscreen(bool fullscreen) {
	//set window style
	DWORD style = fullscreen ? (WS_POPUP | WS_VISIBLE) : WINDOWMODE_PROPERTIES;
	SetWindowLong(hWindow, GWL_STYLE, style);

	if (not fullscreen) {
		ChangeDisplaySettings(nullptr, 0);

		_adjustWindow(); //reset back
	}
	else {
		DEVMODE dm;
		memset(&dm, 0, sizeof(dm));
		dm.dmSize = sizeof(dm);
		dm.dmPelsWidth = windowWidth;
		dm.dmPelsHeight = windowHeight;
		dm.dmBitsPerPel = 32;
		dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		LONG ret = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

		CHECK_WIN32_ERROR(ret == DISP_CHANGE_SUCCESSFUL, "while setting fullscreen mode");

		//WARNING MoveWindow can change backbuffer size
		MoveWindow(hWindow, 0, 0, dm.dmPelsWidth, dm.dmPelsHeight, TRUE);

		clientAreaYOffset = 0;
	}

	ShowCursor(not fullscreen);
}

void Win32Platform::setFullscreen(bool fullscreen) {
	if (fullscreen == mFullscreen) {
		return;
	}

	_setFullscreen(fullscreen);

	mFullscreen = fullscreen;

	//store the new setting into config.ds
	config.set("fullscreen", mFullscreen);
	save(config, "config");
}

wchar_t szPath[MAX_PATH];

utf::string getSystemFolderPath(DWORD folderID) {
	SHGetFolderPathW(
		hWindow,
		folderID | CSIDL_FLAG_CREATE,
		nullptr,
		0,
		szPath);

	std::wstring path(szPath);
	return Path::makeCanonical(String::toUTF8(path));
}

void Win32Platform::initialize(Unique<Game> g) {

	DEBUG_ASSERT(g, "The Game implementation passed to initialize() can't be null");

	game = std::move(g);

	//init appdata folder
	auto cleanName = game->getName().copy();
	Path::removeInvalidChars(cleanName);
	mAppDataPath = getSystemFolderPath(CSIDL_APPDATA) + cleanName + '/';
	//create the appdata user folder
	CreateDirectoryW(String::toUTF16(mAppDataPath).c_str(), nullptr);

	//get root path
	GetCurrentDirectoryW(MAX_PATH, szPath);
	mRootPath = Path::makeCanonical(String::toUTF8(szPath));

	GetTempPathW(MAX_PATH, szPath);
	mShaderCachePath = Path::makeCanonical(String::toUTF8(std::wstring(szPath)) + "dojoshadercache");
	CreateDirectoryW(String::toUTF16(mAppDataPath).c_str(), nullptr);

	DEBUG_MESSAGE("Initializing Dojo Win32");


	//load settings
	auto userConfig = Table::loadFromFile(mRootPath + "/config.ds");

	if (userConfig.isEmpty()) { //also look in appdata
		userConfig = Table::loadFromFile(getAppDataPath() + "/config.ds");
	}

	config.inherit(&userConfig); //use the table that was loaded from file but override any config-passed members

	auto w = std::min(screenWidth, game->getNativeWidth());
	auto h = std::min(screenHeight, game->getNativeHeight());

	Vector windowSize = config.getVector("windowSize", Vector((float)w, (float)h));
	windowWidth = (uint32_t)windowSize.x;
	windowHeight = (uint32_t)windowSize.y;

	//a window can be fullscreen only if the windowSize equals the screenSize, and if it wants to
	mFullscreen = windowWidth == screenWidth and windowHeight == screenHeight and config.getBool("fullscreen");

	//just use the game's preferred settings
	if (not _initializeWindow(game->getName(), windowWidth, windowHeight)) {
		return;
	}

	setVSync(config.getBool("disable_vsync") ? 0 : 1);

	render = make_unique<Renderer>(
		RenderSurface{
			windowWidth,
			windowHeight,
			PixelFormat::RGBA_8_8_8_8 }, //TODO actually read what format it is
		DO_LANDSCAPE_LEFT
	);

	sound = make_unique<SoundManager>();

	input = make_unique<InputSystem>();

	//setup mouse
	POINT mousePos;
	GetCursorPos(&mousePos);
	prevCursorPos.x = (float)mousePos.x;
	prevCursorPos.y = (float)mousePos.y;

	//add the keyboard
	input->addDevice(mKeyboard);

	//create xinput persistent joysticks
	for (int i = 0; i < 4; ++i) {
		mXInputJoystick[i] = make_unique<XInputController>(i);
		mXInputJoystick[i]->poll(1); //force detection of already connected pads
	}

	fonts = make_unique<FontSystem>();

	//mBackgroundQueue = new BackgroundQueue( config.getInt( "threads", -1) );

	DEBUG_MESSAGE("---- Game Launched!");

	//start the game
	game->begin();
}

void Win32Platform::prepareThreadContext() {
	auto job = make_shared<std::promise<HGLRC>>();
	auto futureHandle = job->get_future();

	//queue this on the main thread
	getMainThreadPool().queue([job] {
		auto context = wglCreateContext(hdc);

		bool success = wglShareLists(hglrc, context) != 0;

		CHECK_WIN32_ERROR(success, "while creating and sharing a new context");

		//signal the caller
		job->set_value(context);
	});
	auto contextHandle = futureHandle.get();

	//be nice, wglMakeCurrent just wants you to ask politely and more than once
	//when used in multithreading context, it will randomly fail once in 7/8 tries, just wait and keep on trying
	int tries = 0;
	for (; wglMakeCurrent(hdc, contextHandle) == FALSE and tries < 1000; ++tries) {
		std::this_thread::yield();
	}

	DEBUG_ASSERT(tries < 1000, "Cannot share OpenGL on this thread");
}

void Win32Platform::shutdown() {
	//get out of fullscreen
	_setFullscreen(false);

	if (game) {
		game->end();
		game = {};
	}

	// and a cheesy fade exit
	AnimateWindow(hWindow, 200, AW_HIDE | AW_BLEND);
}

void Win32Platform::acquireContext() {
	wglMakeCurrent(hdc, hglrc);
}

void Win32Platform::submitFrame() {
	SwapBuffers(hdc);
}

void Win32Platform::_pollDevices(float dt) {
	input->poll(dt);

	for (auto&& j : mXInputJoystick) { //poll disconnected pads for connection
		if (not j->isConnected()) {
			j->poll(dt);
		}
	}

	//TODO DInput joysticks
}

void Win32Platform::step(float dt) {
	mStepTimer.reset();

	//update input
	_pollDevices(dt);

	game->loop(dt);

	sound->update(dt);

	render->renderFrame(dt);

	_runASyncTasks((float)mStepTimer.getElapsedTime());

	//take the time before swapBuffers because on some implementations it is blocking
	realFrameTime = (float)mStepTimer.getElapsedTime();

	render->endFrame(); //present the frame
}

void Win32Platform::loop() {
	DEBUG_ASSERT(game, "A game must be specified when starting the main loop");

	frameInterval = game->getNativeFrameLength();

	frameTimer.reset();

	Timer timer;
	running = true;

	MSG msg;
	while (running and game->isRunning()) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				self._fireTermination();
				running = false;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (running) {
			//never send a dt lower than the minimum!
			float dt = std::min(game->getMaximumFrameLength(), (float)timer.getAndReset());

			if (not mFrameSteppingEnabled or (mFrameSteppingEnabled and mFramesToAdvance > 0)) {
				step(dt);

				if (mFrameSteppingEnabled) {
					--mFramesToAdvance;
				}
			}
		}
	}
}

const KeyCode touchTypeToKeyMap[] = {
	KC_UNASSIGNED, // 	TT_HOVER,
	KC_UNASSIGNED, // 	TT_TAP,
	KC_MOUSE_LEFT, // 	TT_LEFT_CLICK,
	KC_MOUSE_RIGHT, // 	TT_RIGHT_CLICK,
	KC_MOUSE_MIDDLE // 	TT_MIDDLE_CLICK
};

void Win32Platform::mousePressed(int cx, int cy, Touch::Type type) {
	//TODO use the button ID!
	mMousePressed = true;
	dragging = true;

	cursorPos.x = (float)cx;
	cursorPos.y = (float)cy - clientAreaYOffset;

	input->_fireTouchBeginEvent(cursorPos, type);

	//small good-will hack- map the mouse keys on the keyboard!
	mKeyboard._notifyButtonState(touchTypeToKeyMap[(int)type], true);
}

void Win32Platform::mouseWheelMoved(int wheelZ) {
	input->_fireScrollWheelEvent((float)wheelZ);
}

void Win32Platform::mouseMoved(int cx, int cy) {
	cursorPos.x = (float)cx;
	cursorPos.y = (float)cy - clientAreaYOffset;

	if (realMouseEvent) {
		if (dragging) {
			input->_fireTouchMoveEvent(cursorPos, prevCursorPos, Touch::Type::LeftClick);    //TODO this doesn't really work but Win doesn't tell
		}
		else {
			input->_fireMouseMoveEvent(cursorPos, prevCursorPos);
		}
	}

	prevCursorPos = cursorPos;

	if (realMouseEvent) {
		if (mouseLocked) { //put the cursor back in the center
			realMouseEvent = false; //setcursor will cause another mouseMoved event, avoid to trigger a loop!
			POINT center = { (LONG)windowWidth / 2, (LONG)windowWidth / 2 };
			ClientToScreen(hWindow, &center);
			SetCursorPos(center.x, center.y);
		}
	}
	else {
		realMouseEvent = true;
	}
}

void Win32Platform::mouseReleased(int cx, int cy, Touch::Type type) {
	//windows can actually send "released" messages whose "pressed" event was sent to another window
	//or used to awake the current one - send a fake mousePressed event if this happens!
	if (not mMousePressed) {
		mousePressed(cx, cy, type);
	}

	mMousePressed = false;
	dragging = false;

	cursorPos.x = (float)cx;
	cursorPos.y = (float)cy - clientAreaYOffset;

	input->_fireTouchEndEvent(cursorPos, type);

	//small good-will hack- map the mouse keys on the keyboard!
	mKeyboard._notifyButtonState(touchTypeToKeyMap[(int)type], false);
}

void Win32Platform::setMouseLocked(bool locked) {
	if (mouseLocked != locked) {

		ShowCursor(not locked);
		ShowCursor(not locked);

		mouseLocked = locked;
	}
}

void Win32Platform::setVSync(int interval/*=1*/) {
	auto func = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if (func == nullptr) {
		DEBUG_MESSAGE("Warning: \"WGL_EXT_swap_control\" extension not supported on your computer, disabling vsync");
		return; // Error: WGL_EXT_swap_control extension not supported on your computer.\n");
	}

	func(interval);
}


void Win32Platform::keyPressed(int kc) {
	//TODO reimplement text!
	lastPressedText = 0;

	KeyCode key = mKeyMap[kc];

#ifdef _DEBUG

	if (key == KC_DIVIDE) {
		mFrameSteppingEnabled = not mFrameSteppingEnabled;
	}
	else if (mFrameSteppingEnabled) {
		if (key == KC_0) {
			mFramesToAdvance = 1;
		}
		else if (key == KC_1) {
			mFramesToAdvance = 5;
		}
		else if (key == KC_2) {
			mFramesToAdvance = 10;
		}
		else if (key == KC_3) {
			mFramesToAdvance = 20;
		}
		else if (key == KC_4) {
			mFramesToAdvance = 50;
		}
		else if (key == KC_5) {
			mFramesToAdvance = 100;
		}
		else if (key == KC_6) {
			mFramesToAdvance = 200;
		}
		else if (key == KC_7) {
			mFramesToAdvance = 500;
		}
	}

	if (mFramesToAdvance) { //capture the input
		return;
	}

#endif

	mKeyboard._notifyButtonState(key, true);
}

void Win32Platform::keyReleased(int kc) {
	mKeyboard._notifyButtonState(mKeyMap[kc], false);
}

PixelFormat Win32Platform::loadImageFile(std::vector<uint8_t>& imageData, utf::string_view path, uint32_t& width, uint32_t& height, int& pixelSize) {
	//pointer to the image, once loaded
	FIBITMAP* dib = nullptr;

	//I know that FreeImage can deduce the fif from file, but I want to enforce correct filenames
	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilenameU(String::toUTF16(path).c_str());

	//if still unkown, return failure
	if (fif == FIF_UNKNOWN) {
		if (Path::getFileExtension(path) == "img") {
			fif = FIF_PNG;
		}
		else {
			return PixelFormat::Unknown;
		}
	}

	//check that the plugin has reading capabilities and load the file
	if (not FreeImage_FIFSupportsReading(fif)) {
		return PixelFormat::Unknown;
	}

	auto buf = loadFileContent(path);

	// attach the binary data to a memory stream
	FIMEMORY* hmem = FreeImage_OpenMemory(buf.data(), static_cast<DWORD>(buf.size()));

	// load an image from the memory stream
	dib = FreeImage_LoadFromMemory(fif, hmem, 0);

	//if the image failed to load, return failure
	if (not dib) {
		return PixelFormat::Unknown;
	}

	//retrieve the image data
	byte* data = (byte*)FreeImage_GetBits(dib);

	//get the image width and height, and size per pixel
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);

	pixelSize = FreeImage_GetBPP(dib) / 8;

	DEBUG_ASSERT(pixelSize == 3 or pixelSize == 4, "Error: Only RGB and RGBA images are supported!");

	uint32_t size = pitch * height;
	imageData.resize(size);

	{
		byte* in, *out;

		for (uint32_t ii, i = 0; i < height; ++i) {
			ii = height - i - 1;

			for (uint32_t j = 0; j < width; ++j) {
				out = imageData.data() + j * pixelSize + i * pitch;
				in = (byte*)data + j * pixelSize + ii * pitch;

				if (pixelSize >= 4) {
					out[3] = in[3];
				}

				if (pixelSize >= 3) {
					out[2] = in[0];
					out[1] = in[1];
					out[0] = in[2];
				}
				else {
					out[0] = in[0];
				}
			}
		}
	}

	//free resources
	FreeImage_Unload(dib);
	FreeImage_CloseMemory(hmem);

	auto meta = load(Path::getMetaFilePathFor(path));

	if (meta.getBool("linear")) {
		return pixelSize == 4 ? PixelFormat::RGBA_8_8_8_8 : PixelFormat::RGB_8_8_8;
	}
	else {
		return pixelSize == 4 ? PixelFormat::RGBA_8_8_8_8_SRGB : PixelFormat::RGB_8_8_8_SRGB;
	}
}

utf::string_view Win32Platform::getAppDataPath() {
	return mAppDataPath;
}

utf::string_view Win32Platform::getRootPath() {
	return mRootPath;
}

utf::string_view Win32Platform::getResourcesPath() {
	return getRootPath(); //on windows, it is the same
}

utf::string_view Win32Platform::getPicturesPath() {
	static auto picturesPath = getSystemFolderPath(CSIDL_MYPICTURES);
	return picturesPath;
}

utf::string_view Win32Platform::getShaderCachePath() {
	return mShaderCachePath;
}

void Win32Platform::openWebPage(utf::string_view site) {
	ShellExecuteW(hWindow, L"open", String::toUTF16(site).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

//init key map
void Win32Platform::_initKeyMap() {
	ZeroMemory(mKeyMap, sizeof(mKeyMap));

	mKeyMap[VK_ESCAPE] = KC_ESCAPE;
	mKeyMap[VK_BACK] = KC_BACK;
	mKeyMap[VK_TAB] = KC_TAB;

	mKeyMap[0x30] = KC_0;
	mKeyMap[0x31] = KC_1;
	mKeyMap[0x32] = KC_2;
	mKeyMap[0x33] = KC_3;
	mKeyMap[0x34] = KC_4;
	mKeyMap[0x35] = KC_5;
	mKeyMap[0x36] = KC_6;
	mKeyMap[0x37] = KC_7;
	mKeyMap[0x38] = KC_8;
	mKeyMap[0x39] = KC_9;

	mKeyMap[0x41] = KC_A;
	mKeyMap[0x42] = KC_B;
	mKeyMap[0x43] = KC_C;
	mKeyMap[0x44] = KC_D;
	mKeyMap[0x45] = KC_E;
	mKeyMap[0x46] = KC_F;
	mKeyMap[0x47] = KC_G;
	mKeyMap[0x48] = KC_H;
	mKeyMap[0x49] = KC_I;
	mKeyMap[0x4A] = KC_J;
	mKeyMap[0x4B] = KC_K;
	mKeyMap[0x4C] = KC_L;
	mKeyMap[0x4D] = KC_M;
	mKeyMap[0x4E] = KC_N;
	mKeyMap[0x4F] = KC_O;
	mKeyMap[0x50] = KC_P;
	mKeyMap[0x51] = KC_Q;
	mKeyMap[0X52] = KC_R;
	mKeyMap[0X53] = KC_S;
	mKeyMap[0X54] = KC_T;
	mKeyMap[0X55] = KC_U;
	mKeyMap[0X56] = KC_V;
	mKeyMap[0X57] = KC_W;
	mKeyMap[0X58] = KC_X;
	mKeyMap[0X59] = KC_Y;
	mKeyMap[0X5A] = KC_Z;

	mKeyMap[VK_NUMPAD0] = KC_NUMPAD0;
	mKeyMap[VK_NUMPAD1] = KC_NUMPAD1;
	mKeyMap[VK_NUMPAD2] = KC_NUMPAD2;
	mKeyMap[VK_NUMPAD3] = KC_NUMPAD3;
	mKeyMap[VK_NUMPAD4] = KC_NUMPAD4;
	mKeyMap[VK_NUMPAD5] = KC_NUMPAD5;
	mKeyMap[VK_NUMPAD6] = KC_NUMPAD6;
	mKeyMap[VK_NUMPAD7] = KC_NUMPAD7;
	mKeyMap[VK_NUMPAD8] = KC_NUMPAD8;
	mKeyMap[VK_NUMPAD9] = KC_NUMPAD9;

	mKeyMap[VK_F1] = KC_F1;
	mKeyMap[VK_F2] = KC_F2;
	mKeyMap[VK_F3] = KC_F3;
	mKeyMap[VK_F4] = KC_F4;
	mKeyMap[VK_F5] = KC_F5;
	mKeyMap[VK_F6] = KC_F6;
	mKeyMap[VK_F7] = KC_F7;
	mKeyMap[VK_F8] = KC_F8;
	mKeyMap[VK_F9] = KC_F9;
	mKeyMap[VK_F10] = KC_F10;
	mKeyMap[VK_F11] = KC_F11;
	mKeyMap[VK_F12] = KC_F12;
	mKeyMap[VK_F13] = KC_F13;
	mKeyMap[VK_F14] = KC_F14;
	mKeyMap[VK_F15] = KC_F15;

	mKeyMap[VK_OEM_MINUS] = KC_MINUS;
	mKeyMap[0] = KC_EQUALS;

	mKeyMap[0] = KC_LBRACKET;
	mKeyMap[0] = KC_RBRACKET;
	mKeyMap[VK_RETURN] = KC_RETURN;
	mKeyMap[VK_LCONTROL] = KC_LCONTROL;

	mKeyMap[0] = KC_SEMICOLON;
	mKeyMap[0] = KC_APOSTROPHE;
	mKeyMap[0] = KC_GRAVE;
	mKeyMap[VK_SHIFT] = KC_LSHIFT;
	mKeyMap[0] = KC_BACKSLASH;

	mKeyMap[0] = KC_COMMA;
	mKeyMap[VK_OEM_PERIOD] = KC_PERIOD;
	mKeyMap[0] = KC_SLASH;
	mKeyMap[VK_RSHIFT] = KC_RSHIFT;
	mKeyMap[VK_MULTIPLY] = KC_MULTIPLY;
	mKeyMap[18] = KC_LEFT_ALT;
	mKeyMap[VK_SPACE] = KC_SPACE;
	mKeyMap[VK_CAPITAL] = KC_CAPITAL;


	mKeyMap[VK_NUMLOCK] = KC_NUMLOCK;
	mKeyMap[VK_SCROLL] = KC_SCROLL;


	mKeyMap[VK_SUBTRACT] = KC_SUBTRACT;
	mKeyMap[VK_ADD] = KC_ADD;
	mKeyMap[VK_DECIMAL] = KC_DECIMAL;

	mKeyMap[VK_OEM_102] = KC_OEM_102;


	mKeyMap[0] = KC_KANA;
	mKeyMap[0] = KC_ABNT_C1;
	mKeyMap[0] = KC_CONVERT;
	mKeyMap[0] = KC_NOCONVERT;
	mKeyMap[0] = KC_YEN;
	mKeyMap[0] = KC_ABNT_C2;
	mKeyMap[0] = KC_NUMPADEQUALS;

	mKeyMap[0] = KC_PREVTRACK;
	mKeyMap[0] = KC_AT;
	mKeyMap[0] = KC_COLON;
	mKeyMap[VK_CANCEL] = KC_STOP;
	mKeyMap[0] = KC_NUMPADENTER;
	mKeyMap[0] = KC_RCONTROL;

	mKeyMap[VK_VOLUME_MUTE] = KC_MUTE;
	mKeyMap[VK_VOLUME_DOWN] = KC_VOLUMEDOWN;
	mKeyMap[VK_VOLUME_UP] = KC_VOLUMEUP;
	mKeyMap[VK_OEM_COMMA] = KC_NUMPADCOMMA;
	mKeyMap[VK_DIVIDE] = KC_DIVIDE;
	mKeyMap[17] = KC_RIGHT_ALT;
	mKeyMap[VK_PAUSE] = KC_PAUSE;

	mKeyMap[VK_HOME] = KC_HOME;
	mKeyMap[VK_PRIOR] = KC_PGUP;
	mKeyMap[VK_NEXT] = KC_PGDOWN;
	mKeyMap[VK_INSERT] = KC_INSERT;
	mKeyMap[VK_DELETE] = KC_DELETE;

	mKeyMap[VK_UP] = KC_UP;
	mKeyMap[VK_LEFT] = KC_LEFT;
	mKeyMap[VK_RIGHT] = KC_RIGHT;
	mKeyMap[VK_DOWN] = KC_DOWN;

	mKeyMap[VK_END] = KC_END;

	mKeyMap[VK_LWIN] = KC_LWIN;
	mKeyMap[VK_RWIN] = KC_RWIN;
	mKeyMap[VK_APPS] = KC_APPS;
	mKeyMap[VK_SLEEP] = KC_SLEEP;

	mKeyMap[0] = KC_UNASSIGNED;
}
