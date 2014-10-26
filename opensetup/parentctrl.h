// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2014 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _PARENTCTRL_H_
#define _PARENTCTRL_H_

class CParentCtrl
{
private:
    typedef HANDLE (WINAPI* LPFNCREATETOOLHELP32SNAPSHOT)(DWORD,DWORD);
    typedef BOOL (WINAPI* LPFNPROCESS32FIRSTNEXT)(HANDLE,void*);

    HANDLE m_hParent;

public:
    CParentCtrl();
    ~CParentCtrl();
    //
    bool IsAvail();
    bool Kill();
};

#endif  /* _PARENTCTRL_H_ */
