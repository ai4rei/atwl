// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010 Ai4rei/AN
// See doc/license.txt for details.
// -----------------------------------------------------------------

#ifndef _TAB_H_
#define _TAB_H_

class CTabMgr
{
private:
    HIMAGELIST hImgList;
    HINSTANCE hInstance;
    HWND hWnd, hWndParent;
public:
    ~CTabMgr();
    bool __stdcall Init(HINSTANCE hInstance, HWND hWndParent);
    void __stdcall Kill(void);
    HWND __stdcall AddTab(const char* lpszCaption, int nCmd, int nImgId, bool bEnabled, const char* lpszRes, DLGPROC lpfnProc);
    void __stdcall AddButton(const char* lpszCaption, int nCmd, int nImgId);
};

#endif  /* _TAB_H_ */
