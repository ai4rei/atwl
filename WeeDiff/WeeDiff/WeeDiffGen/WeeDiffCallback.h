#ifndef _WEE_DIFF_CALLBACK_H
#define _WEE_DIFF_CALLBACK_H

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

#include "..\Common\WeeDiffGenPlugin.h"

#include "RagExe.h"
#include "WeeInputBox.h"
#include "WeeUtility.h"

#include <algorithm>

class WeeDiffCallback : public WeeDiffGenPlugin::IWDGCallback
{
public:

	virtual void LogMsg(LPCSTR lpszMsg);
	virtual INT32 DisplayMessageBox(LPCTSTR lpszCaption, LPCTSTR lpszText, LPCTSTR lpszCheckbox, UINT32 uIcondIndex, INT32 iStyle) { return m_gui->DisplayMessageBox(lpszCaption, lpszText, lpszCheckbox, uIcondIndex, iStyle); }
	virtual UINT32 DisplayInputBox(LPCTSTR lpszCaption, LPCTSTR lpszText, LPTSTR lpchDst, UINT32 uDstSize)  { return WeeInputBox::DisplayInputBox(m_gui->GetMainHandle(), lpszCaption, lpszText, lpchDst, uDstSize, m_hFont); }
	virtual void UpdateListView() { HWND hDlgItem = GetDlgItem(m_gui->GetMainHandle(), 1000); InvalidateRect(hDlgItem, NULL, FALSE); }
	virtual UINT32 Match(WeeDiffGenPlugin::LPFINDDATA lpFindData) { return (*m_ragexe)->Match(lpFindData); }
	virtual void Matches(WeeDiffGenPlugin::fnCBAddOffset CBAddOffset, WeeDiffGenPlugin::LPFINDDATA lpFindData) { return (*m_ragexe)->Matches(CBAddOffset, lpFindData); }
	virtual BYTE GetBYTE(UINT32 uOffset) { return (*m_ragexe)->GetBYTE(uOffset); }
	virtual WORD GetWORD(UINT32 uOffset) { return (*m_ragexe)->GetWORD(uOffset); }
	virtual DWORD32 GetDWORD32(UINT32 uOffset) { return (*m_ragexe)->GetDWORD32(uOffset); }
	virtual INT32 Read(UINT32 uOffset, UCHAR *pBuffer, UINT32 uSize) { return (*m_ragexe)->Read(uOffset, pBuffer, uSize); }
	virtual INT32 Replace(WeeDiffGenPlugin::fnCBAddDiffData CBAddDiffData, UINT32 uOffset, WeeDiffGenPlugin::LPFINDDATA lpFindData, bool bZeroTerminate = false) { return (*m_ragexe)->Replace(CBAddDiffData, uOffset, lpFindData, bZeroTerminate); }
	virtual UINT32 FindStr(WeeDiffGenPlugin::LPFINDDATA lpFindData, bool bReturnRva = false) { return (*m_ragexe)->FindStr(lpFindData, bReturnRva); }
	virtual void GetSection(CCHAR *lpszSectionName, PIMAGE_SECTION_HEADER lpImageSectionHeader) { return (*m_ragexe)->GetSection(lpszSectionName, lpImageSectionHeader); }
	virtual void GetDOSHeader(PIMAGE_DOS_HEADER lpImageDosHeader) { return (*m_ragexe)->GetDOSHeader(lpImageDosHeader); }
	virtual void GetNTHeaders(PIMAGE_NT_HEADERS lpImageNTHeaders) { return (*m_ragexe)->GetNTHeaders(lpImageNTHeaders); }
	virtual UINT32 GetNextFreeOffset(UINT32 uSize) { return (*m_ragexe)->GetNextFreeOffset(uSize); }
	virtual UINT32 Raw2Rva(UINT32 uOffset) { return (*m_ragexe)->Raw2Rva(uOffset); }
	virtual UINT32 Rva2Raw(UINT32 uOffset) { return (*m_ragexe)->Rva2Raw(uOffset); }
	virtual UINT32 FindFunction(CHAR *lpszFunctionName) { return (*m_ragexe)->FindFunction(lpszFunctionName); }
	virtual UINT32 GetClientDate() { return (*m_ragexe)->GetClientDate(); }

	//////////////////////////////////////////////////////////////////////////

	WeeDiffCallback(WeePlugin::IGUI *gui, RagExe **re);

	~WeeDiffCallback()
	{
		DeleteObject(m_hFont);
	}

private:
	RagExe **m_ragexe;
	WeePlugin::IGUI *m_gui;
	HFONT m_hFont;
};

#endif // _WEE_DIFF_CALLBACK_H