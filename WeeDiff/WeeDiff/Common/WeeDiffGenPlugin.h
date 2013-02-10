#ifndef _WEE_DIFFGEN_PLUGIN_H
#define _WEE_DIFFGEN_PLUGIN_H

/************************************************************************/
/*	WeeDiffPlugin.h
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

#include <Windows.h>

#include "WeeDiffPlugin.h"

// Masks
enum
{
	WFD_PATTERN  = 0x1,
	WFD_WILDCARD = 0x2,
	WFD_SECTION  = 0x4,
};

namespace WeeDiffGenPlugin
{

#pragma pack(push, 1)

	typedef struct _WDGPLUGININFO
	{
		LPTSTR lpszDiffName;
		LPTSTR lpszDiffDesc;
		LPTSTR lpszDiffType;		
		LPTSTR lpszDiffGroup; // Deprecated and will be probably replaced with auto-detector for diff collisions.
		LPTSTR lpszAuthorName;
		USHORT unMinMajorVersion;
		USHORT unMinMinorVersion;
		GUID guid; // Will be also used to store information about auto-saving.
		LPTSTR lpszNote;
	} 
	WDGPLUGININFO, *LPWDGPLUGININFO;

	typedef struct _DIFFDATA
	{
		INT32 iOffset;
		UCHAR iReplaceValue;
	}
	DIFFDATA, *LPDIFFDATA;

	/************************************************************************/
	/* Used to search and replace data.
	/* When you mask it with WFD_PATTERN, you don't have to set uDataSize,
	/* size it will be converted by its own. E.g. "09 AB CD EF ..." will result in
	/* 0x90ABCDEF ...
	/*
	/* You can, however, combine strings and pattern conversion together. Just suround the string with ''.
	/* E.g. "09 AB 'ABC' CD EF" will result in 0x09AB414243CDEF
	/*
	/* You can also specify a wild card through chWildCard in combination with WFD_WILDCARD.
	/* If you want to search only in a specific section, then set WFD_SECTION and point lpszSection
	/* to a section.
	/*
	/* uStart and uFinish set the border offsets to search within, both of them being
	/* absolute image offsets. The effective search area is (uStart;uFinish-1). If you
	/* use WFD_SECTION, these values are ignored.
	/*
	/************************************************************************/
	typedef struct _FINDDATA
	{
		UINT32 uMask;
		CHAR *lpData;
		UINT32 uDataSize;
		CHAR chWildCard;
		UINT32 uStart;
		UINT32 uFinish;
		CHAR *lpszSection;
	}
	FINDDATA, *LPFINDDATA;

#pragma pack(pop)

/************************************************************************/
/* Notification messages sent from the diff generator.
/************************************************************************/
enum {
	NM_ENABLED,
	NM_DISABLED
};

typedef std::vector<DIFFDATA> DiffData;

/************************************************************************/
/* Well.. at first I though not to use shared dlls which wouldn't have the
/* same runtime manager resulting in illegl allocation/deallocation operations.
/* Those callbacks are there in case someone creates a plug-in which doesn't
/* rely on shared dlls.
/************************************************************************/
typedef void (__stdcall *fnCBAddDiffData)(LPDIFFDATA);
typedef void (__stdcall *fnCBAddOffset)(UINT32);

/************************************************************************/
/* The plug-in will receive a pointer to a class that implements this
/* this interface. It contains functions to generate diffs.
/************************************************************************/
class IWDGCallback
{
public:	
	// At least, each plug-in should be able to log information.
	virtual void LogMsg(LPCSTR lpszMsg) = 0;

	// Displays.. a message box. It works almost the same as the default windows message box except it draws
	// a check box at the bottom of the dialog and who cares, it uses an owner drawn windows vista styled dialog box.
	// To enable the check box you have to add MB_CHECKBOX to the style parameter.
	// Use a combination of MB_TYPEMASK and IDCHECKBOX to see if check box was active and what message was returned.
	virtual INT32 DisplayMessageBox(LPCTSTR lpszCaption, LPCTSTR lpszText, LPCTSTR lpszCheckbox, UINT32 uIcondIndex, INT32 iStyle) = 0;

	// Displays.. an input box! Yay. However, it will fill lpchDst with data entered by the user.
	virtual UINT32 DisplayInputBox(LPCTSTR lpszCaption, LPCTSTR lpszText, LPTSTR lpchDst, UINT32 uDstSize) = 0;

	// Refreshes the list view.
	virtual void UpdateListView() = 0;

	// Returns the offset that matches the data set in lpFindData.
	virtual UINT32 Match(LPFINDDATA lpFindData) = 0;

	// Calls CBAddOffset as long as a match has been found.
	virtual void Matches(WeeDiffGenPlugin::fnCBAddOffset CBAddOffset, LPFINDDATA lpFindData) = 0;

	// Self explanatory.
	virtual BYTE GetBYTE(UINT32 uOffset) = 0;
	virtual WORD GetWORD(UINT32 uOffset) = 0;
	virtual DWORD32 GetDWORD32(UINT32 uOffset) = 0;

	// Copies data at uOffset into pBuffer and uSize.
	virtual INT32 Read(UINT32 uOffset, UCHAR *pBuffer, UINT32 uSize) = 0;

	// Calls CBAddDiffData each time a diff was found. Optionally you can set bZeroTerminate to true so that
	// a zero is appended afterwards.
	virtual INT32 Replace(WeeDiffGenPlugin::fnCBAddDiffData CBAddDiffData, UINT32 uOffset, LPFINDDATA lpFindData, bool bZeroTerminate = false) = 0;

	// Returns the offset of a string. It will only searches in .rdata section since strings usually reside there.
	virtual UINT32 FindStr(LPFINDDATA lpFindData, bool bReturnRva = false) = 0;

	// Searches a section with name lpszSectionName and fills the lpImageSectionHeader structure.
	virtual void GetSection(CCHAR *lpszSectionName, PIMAGE_SECTION_HEADER lpImageSectionHeader) = 0;

	// Same as above, this time with DOS header.
	virtual void GetDOSHeader(PIMAGE_DOS_HEADER lpImageDosHeader) = 0;

	// Same as above, this time with NT headers.
	virtual void GetNTHeaders(PIMAGE_NT_HEADERS lpImageNTHeaders) = 0;

	// The diff generator always creates a section named ".diff" with a raw size of 1024 bytes and virtual size
	// of 4096 size. This function returns a pointer to the unused space. Internally it saves the last used offset
	// to prevent collisions.
	virtual UINT32 GetNextFreeOffset(UINT32 uSize) = 0;

	// Converts the raw offset to the equivalent rva offset.
	virtual UINT32 Raw2Rva(UINT32 uOffset) = 0;

	// Same es above, vice versa.
	virtual UINT32 Rva2Raw(UINT32 uOffset) = 0;

	// Returns the offset of a function name by looking up the import table.
	virtual UINT32 FindFunction(CHAR *lpszFunctionName) = 0;

	// Returns the date of the client. E.g. 20101126 for 2010-11-26aRagexeRE.
	virtual UINT32 GetClientDate() = 0;
};

class IWDGPlugin
{
public:

	IWDGPlugin(LPVOID lpData)
	{
		m_dgc = (IWDGCallback *)lpData;
	}

	// Called when the plug-in is being unloaded.
	virtual void Release() = 0;

	// Called whenever data allocated by this plug-in will be deleted.
	virtual void Free(LPVOID memory) = 0;
	
	// Returns information about this plug-in.
	virtual LPWDGPLUGININFO GetPluginInfo() = 0;

	// Called whenever this plug-in has been enabled.
	virtual INT32 Enabled() = 0;

	// Called whenever this plug-in has been disabled.
	virtual INT32 Disabled() = 0;

	// Returns a pointer to the value associated with this plug-in. (GetInputBox)
	// If no input box has been used, this value can be NULL.
	virtual LPCTSTR GetInputValue() = 0;

	// Self-explanatory.
	virtual DiffData *GeneratePatch() = 0;

	// Get a pointer to the diff data.
	virtual DiffData *GetDiffData() = 0;

	// Pointer to the callback interface.
	IWDGCallback *m_dgc;
};

// Each plug-in must export the InitPlugin function.
// I don't care what kind of initialization you will use
// as long as you return a pointer to the class that derivered
// from the IPlugin interface.
typedef IWDGPlugin *(*fnInitPlugin)(LPVOID, USHORT, USHORT);

}

#endif // _WEE_DIFFGEN_PLUGIN_H