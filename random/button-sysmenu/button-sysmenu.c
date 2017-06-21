#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>

#define BUTTON_SYSMENU_WINDOWCLASS _T("ButtonSysMenuWindowClass")

static BOOL CALLBACK WndProcOnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    if(!CreateWindowEx(0, WC_BUTTON, _T("Start"), WS_CHILD|WS_VISIBLE|WS_SYSMENU|BS_PUSHBUTTON, 14, 14, 50, 24, hWnd, NULL, GetWindowInstance(hWnd), NULL))
    {
        return FALSE;
    }

    return TRUE;
}

static void CALLBACK WndProcOnDestroy(HWND hWnd)
{
    PostQuitMessage(0);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        HANDLE_MSG(hWnd, WM_CREATE, &WndProcOnCreate);
        HANDLE_MSG(hWnd, WM_DESTROY, &WndProcOnDestroy);
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpszCmdLine, int nShowCmd)
{
    WNDCLASS Wc = { 0 };

    Wc.lpfnWndProc = &WndProc;
    Wc.lpszClassName = BUTTON_SYSMENU_WINDOWCLASS;
    Wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    Wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
    Wc.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));

    if(RegisterClass(&Wc))
    {
        if(CreateWindowEx(0, BUTTON_SYSMENU_WINDOWCLASS, _T("Focus Button, [Alt] + [-], Move/Close"), WS_VISIBLE|WS_POPUPWINDOW|WS_SYSMENU|WS_CAPTION|WS_MINIMIZEBOX, 0, 0, 320, 240, NULL, NULL, hInstance, NULL))
        {
            MSG Msg;

            while(GetMessage(&Msg, NULL, 0, 0)>0)
            {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }
    }

    return 0;
}
