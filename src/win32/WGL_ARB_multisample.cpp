//-----------------------------------------------------------------------------
// Copyright (c) 2006-2008 dhpoware. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include <cassert>
#include <cstdio>
#include <cstring>
#include "win32/WGL_ARB_multisample.h"

#include "dojo_gl_header.h"

namespace {
	WNDCLASSEX g_wcl;
	HWND g_hWnd;
	HDC g_hDC;
	HGLRC g_hRC;
	char g_szAAPixelFormat[32];

	LRESULT CALLBACK DummyGLWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
		case WM_CREATE:
			g_hDC = GetDC(hWnd);

			if (not g_hDC) {
				return -1;
			}

			break;

		case WM_DESTROY:
			if (g_hDC) {
				if (g_hRC) {
					wglMakeCurrent(g_hDC, 0);
					wglDeleteContext(g_hRC);
					g_hRC = 0;
				}

				ReleaseDC(hWnd, g_hDC);
				g_hDC = 0;
			}

			PostQuitMessage(0);
			return 0;

		default:
			break;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	bool CreateDummyGLWindow() {
		g_wcl.cbSize = sizeof(g_wcl);
		g_wcl.style = CS_OWNDC;
		g_wcl.lpfnWndProc = DummyGLWndProc;
		g_wcl.hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(0));
		g_wcl.lpszClassName = "DummyGLWindowClass";

		if (not RegisterClassEx(&g_wcl)) {
			return false;
		}

		g_hWnd = CreateWindow(g_wcl.lpszClassName, "", WS_OVERLAPPEDWINDOW,
									0, 0, 0, 0, 0, 0, g_wcl.hInstance, 0);

		if (not g_hWnd) {
			return false;
		}

		PIXELFORMATDESCRIPTOR pfd = {0};

		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.cDepthBits = 16;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int pf = ChoosePixelFormat(g_hDC, &pfd);

		if (not SetPixelFormat(g_hDC, pf, &pfd)) {
			return false;
		}

		g_hRC = wglCreateContext(g_hDC);

		if (not g_hRC) {
			return false;
		}

		if (not wglMakeCurrent(g_hDC, g_hRC)) {
			return false;
		}

		return true;
	}

	void ChooseBestCSAAPixelFormat(int& pf) {
		struct CSAAPixelFormat {
			int numColorSamples;
			int numCoverageSamples;
			const char* pszDescription;
		};

		CSAAPixelFormat csaaPixelFormats[] = {
			{4, 8, "8x CSAA"},
			{4, 16, "16x CSAA"},
			{8, 8, "8xQ (Quality) CSAA"},
			{8, 16, "16xQ (Quality) CSAA"}
		};

		int totalCSAAFormats = static_cast<int>(sizeof(csaaPixelFormats) /
				sizeof(CSAAPixelFormat));

		int attributes[] = {
			WGL_SAMPLE_BUFFERS_ARB, 1,
			WGL_COLOR_SAMPLES_NV, 0,
			WGL_COVERAGE_SAMPLES_NV, 0,
			WGL_DOUBLE_BUFFER_ARB, 1,
			0, 0
		};

		int returnedPixelFormat = 0;
		UINT numFormats = 0;
		BOOL bStatus = FALSE;

		for (int i = totalCSAAFormats - 1; i >= 0; --i) {
			attributes[3] = csaaPixelFormats[i].numColorSamples;
			attributes[5] = csaaPixelFormats[i].numCoverageSamples;

			bStatus = wglChoosePixelFormatARB(g_hDC, attributes, 0, 1,
					&returnedPixelFormat, &numFormats);

			if (bStatus == TRUE and numFormats) {
				pf = returnedPixelFormat;
				break;
			}
		}

		if (bStatus == FALSE) {
			g_szAAPixelFormat[0] = '\0';
		}
	}

	void ChooseBestMSAAPixelFormat(int& pf) {
		int attributes[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 24,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			WGL_SAMPLES_ARB, 0,
			0, 0
		};

		int returnedPixelFormat = 0;
		UINT numFormats = 0;
		BOOL bStatus = FALSE;

		for (int samples = 16; samples > 0; samples /= 2) {
			attributes[17] = samples;

			bStatus = wglChoosePixelFormatARB(g_hDC, attributes, 0, 1,
					&returnedPixelFormat, &numFormats);

			if (bStatus == TRUE and numFormats) {
				pf = returnedPixelFormat;
				break;
			}
		}

		if (bStatus == FALSE) {
			g_szAAPixelFormat[0] = '\0';
		}
	}

	void ChooseCSAAPixelFormat(int& pf, int samples) {
		struct CSAAPixelFormat {
			int numColorSamples;
			int numCoverageSamples;
			const char* pszDescription;
		};

		CSAAPixelFormat csaaPixelFormats[] = {
			{4, 16, "16x CSAA"},
			{4, 8, "8x CSAA"}
		};

		CSAAPixelFormat csaaQualityPixelFormats[] = {
			{8, 16, "16xQ (Quality) CSAA"},
			{8, 8, "8xQ (Quality) CSAA"}
		};

		CSAAPixelFormat* pCSAAFormats = 0;

		int attributes[] = {
			WGL_SAMPLE_BUFFERS_ARB, 1,
			WGL_COLOR_SAMPLES_NV, 0,
			WGL_COVERAGE_SAMPLES_NV, 0,
			WGL_DOUBLE_BUFFER_ARB, 1,
			0, 0
		};

		int returnedPixelFormat = 0;
		UINT numFormats = 0;
		BOOL bStatus = FALSE;

		if (samples >= 8) {
			pCSAAFormats = csaaQualityPixelFormats;
		}
		else {
			pCSAAFormats = csaaPixelFormats;
		}

		for (int i = 0; i < 2; ++i) {
			attributes[3] = pCSAAFormats[i].numColorSamples;
			attributes[5] = pCSAAFormats[i].numCoverageSamples;

			bStatus = wglChoosePixelFormatARB(g_hDC, attributes, 0, 1,
					&returnedPixelFormat, &numFormats);

			if (bStatus == TRUE and numFormats) {
				pf = returnedPixelFormat;
				break;
			}
		}

		if (bStatus == FALSE) {
			g_szAAPixelFormat[0] = '\0';
		}
	}

	void ChooseMSAAPixelFormat(int& pf, int samples) {
		int attributes[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 24,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			WGL_SAMPLES_ARB, samples,
			0, 0
		};

		int returnedPixelFormat = 0;
		UINT numFormats = 0;
		BOOL bStatus = wglChoosePixelFormatARB(g_hDC, attributes, 0, 1,
				&returnedPixelFormat, &numFormats);

		if (bStatus == TRUE and numFormats) {
			pf = returnedPixelFormat;
		}
		else {
			g_szAAPixelFormat[0] = '\0';
		}
	}

	void DestroyDummyGLWindow() {
		if (g_hWnd) {
			PostMessage(g_hWnd, WM_CLOSE, 0, 0);

			BOOL bRet;
			MSG msg;

			while ((bRet = GetMessage(&msg, 0, 0, 0)) != 0) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		UnregisterClass(g_wcl.lpszClassName, g_wcl.hInstance);
	}

	bool ExtensionSupported(const char* pszExtensionName) {
		static const char* pszGLExtensions = 0;
		static const char* pszWGLExtensions = 0;

		if (not pszGLExtensions) {
			pszGLExtensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
		}

		if (not pszWGLExtensions) {
			// WGL_ARB_extensions_string.

			typedef const char* (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC);

			PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB =
				reinterpret_cast<PFNWGLGETEXTENSIONSSTRINGARBPROC>(
					wglGetProcAddress("wglGetExtensionsStringARB"));

			if (wglGetExtensionsStringARB) {
				pszWGLExtensions = wglGetExtensionsStringARB(wglGetCurrentDC());
			}
		}

		if (not strstr(pszGLExtensions, pszExtensionName)) {
			if (not strstr(pszWGLExtensions, pszExtensionName)) {
				return false;
			}
		}

		return true;
	}
}

void ChooseBestAntiAliasingPixelFormat(int& pf) {
	pf = 0;

	if (not CreateDummyGLWindow()) {
		DestroyDummyGLWindow();
		return;
	}

	if (ExtensionSupported("GL_NV_multisample_coverage") and
			ExtensionSupported("WGL_NV_multisample_coverage")) {
		ChooseBestCSAAPixelFormat(pf);
	}
	else {
		ChooseBestMSAAPixelFormat(pf);
	}

	DestroyDummyGLWindow();
}

void ChooseAntiAliasingPixelFormat(int& pf, int samples) {
	pf = 0;

	if (not CreateDummyGLWindow()) {
		DestroyDummyGLWindow();
		return;
	}

	if (ExtensionSupported("GL_NV_multisample_coverage") and
			ExtensionSupported("WGL_NV_multisample_coverage")) {
		ChooseCSAAPixelFormat(pf, samples);
	}
	else {
		ChooseMSAAPixelFormat(pf, samples);
	}

	DestroyDummyGLWindow();
}

const char* GetAntiAliasingPixelFormatString() {
	return g_szAAPixelFormat;
}

// WGL_ARB_pixel_format

BOOL wglGetPixelFormatAttribivARB(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, int* piValues) {
	typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBIVARBPROC)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, int* piValues);
	static PFNWGLGETPIXELFORMATATTRIBIVARBPROC pfnGetPixelFormatAttribivARB = 0;

	if (not pfnGetPixelFormatAttribivARB) {
		pfnGetPixelFormatAttribivARB = reinterpret_cast<PFNWGLGETPIXELFORMATATTRIBIVARBPROC>(wglGetProcAddress("wglGetPixelFormatAttribivARB"));
		assert(pfnGetPixelFormatAttribivARB != 0);
	}

	return pfnGetPixelFormatAttribivARB(hdc, iPixelFormat, iLayerPlane, nAttributes, piAttributes, piValues);
}

BOOL wglGetPixelFormatAttribfvARB(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, FLOAT* pfValues) {
	typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBFVARBPROC)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, FLOAT * pfValues);
	static PFNWGLGETPIXELFORMATATTRIBFVARBPROC pfnGetPixelFormatAttribfvARB = 0;

	if (not pfnGetPixelFormatAttribfvARB) {
		pfnGetPixelFormatAttribfvARB = reinterpret_cast<PFNWGLGETPIXELFORMATATTRIBFVARBPROC>(wglGetProcAddress("wglGetPixelFormatAttribfvARB"));
		assert(pfnGetPixelFormatAttribfvARB != 0);
	}

	return pfnGetPixelFormatAttribfvARB(hdc, iPixelFormat, iLayerPlane, nAttributes, piAttributes, pfValues);
}

BOOL wglChoosePixelFormatARB(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats) {
	typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int* piAttribIList, const FLOAT * pfAttribFList, UINT nMaxFormats, int* piFormats, UINT * nNumFormats);
	static PFNWGLCHOOSEPIXELFORMATARBPROC pfnChoosePixelFormatARB = 0;

	if (not pfnChoosePixelFormatARB) {
		pfnChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
		assert(pfnChoosePixelFormatARB != 0);
	}

	return pfnChoosePixelFormatARB(hdc, piAttribIList, pfAttribFList, nMaxFormats, piFormats, nNumFormats);
}
