// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _TAB_H_
#define _TAB_H_

#include <vector>

class CTabMgr
{
private:
    typedef std::vector< std::pair< int, HWND > > TABLIST;

    HIMAGELIST m_hImgList;
    HINSTANCE m_hInstance;
    HWND m_hWnd, m_hWndParent;
    TABLIST m_TabList;

public:
    ~CTabMgr();
    bool __stdcall Init(HINSTANCE hInstance, HWND hWndParent, int nImgDimension, const char* lpszImgList32Id, const char* lpszImgListId, const char* lpszImgListMaskId, int nMaxImages);
    void __stdcall Kill(void);
    HWND __stdcall AddTab(const char* lpszCaption, int nCmd, int nImgId, bool bEnabled, const char* lpszRes, DLGPROC lpfnProc);
    HWND __stdcall GetTab(int nCmd);
    void __stdcall AddButton(const char* lpszCaption, int nCmd, int nImgId);
    void __stdcall OnResizeMove(void);
    void __stdcall OnActivateTab(int nCmd);
    HWND __stdcall GetWindowHandle(void);

private:
    static bool __stdcall HasBrokenTransparencyBitmapSupport(void);
};

#endif  /* _TAB_H_ */
