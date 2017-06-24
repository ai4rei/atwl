#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>

#define TEST_WINDOW_CLASS _T("CloseWindowTest")

static BOOL CALLBACK WndProcOnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    for(;;)
    {
        if(!CreateWindowEx(0, WC_BUTTON, "CloseWindow()", WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON, 14, 14, 100, 24, hWnd, (HMENU)IDOK, GetWindowInstance(hWnd), NULL))
        {
            break;
        }

        SendMessage(GetDlgItem(hWnd, IDOK), WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

        return TRUE;
    }

    return FALSE;

UNREFERENCED_PARAMETER(lpCreateStruct);
}

static void CALLBACK WndProcOnDestroy(HWND hWnd)
{
    PostQuitMessage(0);

UNREFERENCED_PARAMETER(hWnd);
}

static void CALLBACK WndProcOnCommand(HWND hWnd, int nId, HWND hWndCtl, UINT uCodeNotify)
{
    switch(nId)
    {
        case IDOK:
            CloseWindow(hWnd);
            break;
        default:
            FORWARD_WM_COMMAND(hWnd, nId, hWndCtl, uCodeNotify, &DefWindowProc);
            break;
    }
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        HANDLE_MSG(hWnd, WM_CREATE, &WndProcOnCreate);
        HANDLE_MSG(hWnd, WM_DESTROY, &WndProcOnDestroy);
        HANDLE_MSG(hWnd, WM_COMMAND, &WndProcOnCommand);
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpszCmdLine, int nShowCmd)
{
    WNDCLASS Wc = { 0 };

    Wc.lpfnWndProc = &WndProc;
    Wc.lpszClassName = TEST_WINDOW_CLASS;
    Wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    Wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
    Wc.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));

    if(RegisterClass(&Wc))
    {
        if(CreateWindowEx(0, TEST_WINDOW_CLASS, _T("CloseWindow() Test"), WS_VISIBLE|WS_POPUPWINDOW|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX, 100, 100, 320, 240, NULL, NULL, hInstance, NULL))
        {
            MSG Msg;

            while(GetMessage(&Msg, NULL, 0, 0)>0)
            {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }
    }

    return EXIT_SUCCESS;

UNREFERENCED_PARAMETER(hPrevInstance);
UNREFERENCED_PARAMETER(lpszCmdLine);
UNREFERENCED_PARAMETER(nShowCmd);
}
