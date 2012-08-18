#include <stdlib.h>
#include <crtdbg.h>

/************************************************************************/
/*	WDGCustomWindowTitle.cpp
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

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGCustomWindowTitle.h"

WDGPlugin *g_SelfReference = NULL;

void WDGPlugin::Release()
{
	m_diffdata.clear();
	g_SelfReference = NULL;
	delete this;
}

void WDGPlugin::Free(LPVOID memory)
{
	delete memory;
	memory = NULL;
}

LPWDGPLUGININFO WDGPlugin::GetPluginInfo()
{
	static WDGPLUGININFO wpi = 
	{
		TEXT("Custom Window Title"),
		TEXT("Changes the window title to a custom one."),
		TEXT("[UI]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x81784602, 0xca76, 0x457f, { 0xab, 0xc9, 0xad, 0xfe, 0xf3, 0x59, 0xa6, 0x81 } }
	};

	return &wpi;
}

INT32 WDGPlugin::Enabled()
{
	ZeroMemory(m_szValue, MAX_TITLE * sizeof(TCHAR));
	m_dgc->DisplayInputBox(GetPluginInfo()->lpszDiffName, TEXT("Please enter the name for the window title:"), m_szValue, MAX_TITLE);
	m_dgc->UpdateListView();
	GeneratePatch();

	return 0;
}

INT32 WDGPlugin::Disabled()
{
	return 0;
}

LPCTSTR WDGPlugin::GetInputValue()
{
	return m_szValue;
}

DiffData *WDGPlugin::GeneratePatch()
{
	WeeDiffGenPlugin::FINDDATA sFindData = {0};
	CHAR szMsg[256];
	m_diffdata.clear();

	CHAR szTitle[MAX_TITLE + 3];
	ZeroMemory(szTitle, MAX_TITLE + 3);
	CHAR *p = szTitle;

	UINT32 uConvSize = WideCharToMultiByte(CP_ACP, 0, m_szValue, -1, NULL, 0, NULL, NULL);

	if(uConvSize > MAX_TITLE)
	{
		m_dgc->LogMsg("WDGCustomWindowTitle :: Failed to convert wide character to multibyte!");
		return NULL;
	}
	else
	{
		*p++ = '\'';
		if(uConvSize <= 1)
		{
			*p++ = ' ';
		}
		else
		{
			WideCharToMultiByte(CP_ACP, 0, m_szValue, -1, p, MAX_TITLE, NULL, NULL);
			p += uConvSize - 1;
		}
		*p = '\'';

		UINT32 uOffset = 0;
		UINT32 uPart = 1;
		UINT32 uCaptionOffset = m_dgc->GetNextFreeOffset(uConvSize);

		try
		{
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = szTitle;
			sFindData.uMask = WFD_PATTERN;

			m_dgc->Replace(CBAddDiffData, uCaptionOffset, &sFindData, true);

			uPart = 2;

			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "'Ragnarok'";
			sFindData.uMask = WFD_PATTERN;

			uOffset = m_dgc->FindStr(&sFindData, true);

			uPart = 3;

			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = (CHAR *)&uOffset;
			sFindData.uDataSize = 4;
			sFindData.lpszSection = ".text";
			sFindData.uMask = WFD_SECTION;

			uOffset = m_dgc->Match(&sFindData);

			uPart = 4;

			IMAGE_NT_HEADERS sNTHeaders;
			m_dgc->GetNTHeaders(&sNTHeaders);

			uCaptionOffset = m_dgc->Raw2Rva(uCaptionOffset);

			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = (CHAR *)&uCaptionOffset;
			sFindData.uDataSize = 4;

			m_dgc->Replace(CBAddDiffData, uOffset, &sFindData, false);
		} 
		catch (LPCSTR lpszMsg)
		{
			sprintf_s(szMsg, 256, "WDGCustomWindowTitle :: Part %d :: %s", uPart, lpszMsg);
			m_dgc->LogMsg(szMsg);
			return NULL;
		}		
	}

	return &m_diffdata;
}

DiffData *WDGPlugin::GetDiffData()
{
	if(m_diffdata.size() <= 0)
	{
		return NULL;
	}

	return &m_diffdata;
}

extern "C" __declspec(dllexport) WeeDiffGenPlugin::IWDGPlugin *InitPlugin(LPVOID lpData, USHORT unWeeDiffMajorVersion, USHORT unWeeDiffMinorVersion)
{
	// Enable functions to track down memory leaks.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	if(g_SelfReference == NULL)
	{
		g_SelfReference = new WDGPlugin(lpData);
	}

	return g_SelfReference;
}

void WDGPlugin::CBAddDiffData(WeeDiffGenPlugin::LPDIFFDATA lpDiffData)
{
	if(g_SelfReference != NULL)
	{
		g_SelfReference->m_diffdata.push_back(*lpDiffData);
	}
}