// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2014 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _TAB_H_
#define _TAB_H_

#include <vector>

class CTabMgr
{
private:
    typedef std::pair< int, HWND > TABITEM;
    typedef std::vector< TABITEM > TABLIST;

    HIMAGELIST m_hImgList;
    HINSTANCE m_hInstance;
    HWND m_hWnd;
    HWND m_hWndParent;
    TABLIST m_TabList;

protected:
    static bool __stdcall IsTransparencyBitmapSupportBroken();

    void __stdcall InitImgList(int nImgDimension, int nImgListBase, int nMaxImages);
    bool __stdcall GetActiveTab(unsigned long& luIdx);

public:
    CTabMgr();
    ~CTabMgr();

    bool __stdcall Init(HINSTANCE hInstance, HWND hWndParent, int nImgDimension, int nImgListBase, int nMaxImages);
    void __stdcall Kill(void);
    HWND __stdcall AddTab(const char* lpszCaption, int nCmd, int nImgId, bool bEnabled, const char* lpszRes, DLGPROC lpfnProc);
    HWND __stdcall GetTab(int nCmd);
    void __stdcall AddButton(const char* lpszCaption, int nCmd, int nImgId);
    void __stdcall OnResizeMove();
    void __stdcall OnActivateTab(int nCmd, bool bManual = false);
    void __stdcall ActivateTabNext();
    void __stdcall ActivateTabPrev();
    HWND __stdcall GetWindowHandle();
};

#endif  /* _TAB_H_ */
