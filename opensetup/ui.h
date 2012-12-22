// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010 Ai4rei/AN
// See doc/license.txt for details.
// -----------------------------------------------------------------

#ifndef _UI_H_
#define _UI_H_

struct UIBatchList
{
    int nIDDlgItem;
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;
};

class UI
{
public:
    static void __stdcall BatchMessage(HWND hWnd, struct UIBatchList* lpBatchList, unsigned long luItems);
    static void __stdcall FillComboBox(HWND hWnd, int nId, const char** lppszList, unsigned long luItems);
    static int __stdcall MessageBoxEx(HWND hWnd, const char* lpszText, const char* lpszCaption, unsigned long luStyle);
};

#endif  /* _UI_H_ */
