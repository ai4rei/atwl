#include "Window.h"

/************************************************************************/
/*	WeeDiffPlain.c
/*	Copyright (C) 2011 Shinryo
/* 
/*  This software is provided 'as-is', without any express or implied
/*  warranty.  In no event will the authors be held liable for any damages
/*  arising from the use of this software.
/* 
/*	You are allowed to alter this software and redistribute it freely with
/*	the following restrictions:
/*	
/*	1. You must not claim that you wrote the original software.
/*	2. Do not represent your altered source version as original software.
/*	3. This notice may not be removed or altered from any source distribution.
/*	
/************************************************************************/

using namespace WeeTools;

Window::Window(WNDPROC procMessageHandler, LPCTSTR lpszTitle, INT32 x, INT32 y, INT32 cx, INT32 cy, DWORD32 dwClassStyle, DWORD32 dwStyle, DWORD32 dwExStyle)
{
	m_hWnd = NULL;
	m_hMemDC = NULL;

	if(procMessageHandler == NULL)
		throw new WeeException(TEXT(__FUNCTION__) TEXT(": No message handler provided!"), WeeException::E_ERROR);

	m_windowClass.cbSize = sizeof(WNDCLASSEX);
	m_windowClass.style = CS_HREDRAW | CS_VREDRAW | dwClassStyle;
	m_windowClass.lpfnWndProc = procMessageHandler;
	m_windowClass.cbClsExtra = 0;
	m_windowClass.cbWndExtra = 0;
	m_windowClass.hInstance = GetModuleHandle(NULL);
	m_windowClass.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION));
	m_windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	m_windowClass.hbrBackground = GetSysColorBrush(CTLCOLOR_DLG);
	m_windowClass.lpszMenuName = NULL;
	m_windowClass.lpszClassName = lpszTitle;
	m_windowClass.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION));

	if(!RegisterClassEx(&m_windowClass)) 
		throw new WeeException(TEXT(__FUNCTION__) TEXT(": Failed to register class!"), WeeException::E_ERROR);

	m_hWnd = CreateWindowEx(dwExStyle, m_windowClass.lpszClassName, lpszTitle, WS_OVERLAPPEDWINDOW | dwStyle, x, y, cx, cy, GetDesktopWindow(), NULL, m_windowClass.hInstance, NULL);

	if(m_hWnd == NULL)
		throw new WeeException(TEXT(__FUNCTION__) TEXT(": Failed to create new window!"), WeeException::E_ERROR);

	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS) - (osvi.dwMajorVersion >= 6 ? 0 : 4);

	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
	
	AddFont(0, CreateFontIndirect(&ncm.lfMessageFont));
}

Window::~Window()
{
	if(m_hMemDC != NULL) 
	{
		HBITMAP hTmpBmp = CreateCompatibleBitmap(m_hMemDC, 1, 1);
		HBITMAP hBmp = (HBITMAP)SelectObject(m_hMemDC, hTmpBmp);

		if(hBmp != NULL)
		{
			DeleteObject(hBmp);
			DeleteObject(hTmpBmp);
		}

		ReleaseDC(m_hWnd, m_hMemDC);
	}

	for(ControlsMap::iterator it = m_controls.begin(); it != m_controls.end(); it++)
	{
		delete it->second;
	}

	for(FontMap::iterator it = m_fonts.begin(); it != m_fonts.end(); it++) 
		DeleteObject(it->second);

	UnregisterClass(m_windowClass.lpszClassName, m_windowClass.hInstance);
	if(m_hWnd != NULL) {
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}
}

void Window::Center(HWND hWnd)
{
	RECT r;
	SIZE s;

	GetWindowRect(hWnd, &r);

	s.cx = r.right - r.left;
	s.cy = r.bottom - r.top;

	SetWindowPos(hWnd, NULL, (GetSystemMetrics(SM_CXSCREEN) - s.cx)/2, (GetSystemMetrics(SM_CYSCREEN) - s.cy)/2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void Window::UseCommonControls(DWORD32 dwFlags)
{
	INITCOMMONCONTROLSEX icce;

	icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icce.dwICC = dwFlags;
	InitCommonControlsEx(&icce);
}

INT32 Window::MessageLoop(HWND hWnd)
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while(GetMessage(&msg, hWnd, 0, 0))
	{
		//if(!IsDialogMessage(g_hPreviewDlg, &msg))
		//{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		//}
	}

	return (INT32)msg.wParam;
}

POINT Window::GetCenterPoint(HWND hWnd)
{
	RECT r;
	SIZE s;

	GetWindowRect(hWnd, &r);

	s.cx = r.right - r.left;
	s.cy = r.bottom - r.top;

	return GetCenterPoint(s);
}

POINT Window::GetCenterPoint(SIZE s)
{
	POINT p;

	p.x = (GetSystemMetrics(SM_CXSCREEN) - s.cx)/2;
	p.y = (GetSystemMetrics(SM_CYSCREEN) - s.cy)/2;

	return p;
}

HDC Window::GetMemDC()
{
	if(m_hMemDC != NULL)
		return m_hMemDC;

	RECT rcDesktop;

	GetWindowRect(GetDesktopWindow(), &rcDesktop);

	HDC hDC = GetDC(GetDesktopWindow());

	m_hMemDC = CreateCompatibleDC(hDC);
	HBITMAP hMemBmp = CreateCompatibleBitmap(hDC, rcDesktop.right, rcDesktop.bottom);
	SelectObject(m_hMemDC, hMemBmp);

	return m_hMemDC;
}