#ifndef _WEE_DIFF_PLUGIN_H
#define _WEE_DIFF_PLUGIN_H

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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <CommCtrl.h>
#include <vector>

// Used in DiffPatcher's MessageBox handler.

#define MB_CHECKBOX			0x10000000 // Will display a user defined checkbox at the bottom of the message.
#define IDCHECKBOX			0x00010000 // Don't forget to mask the function output with MB_TYPEMASK!

// Icon Index' that are used in the MessageBox handler.
#define MBI_OK				0
#define MBI_WARNING			1
#define MBI_ERROR			2
#define MBI_IMPORTANT		3
#define MBI_QUESTION		4
#define MBI_INFORMATION		5
#define MBI_MAX				6

namespace WeePlugin
{

// All passed struct have to be aligned correctly to work properly.

#pragma pack(push, 1)

	// Basic plug-in information.
	typedef struct _WEEPLUGININFO
	{
		LPTSTR lpszPluginName;
		LPTSTR lpszAuthorName;

		// Don't forget to set proper versions since the DiffPatcher itself could be extended in the future
		// and needs to know what plug-in supports what.
		// Example:
		// Patcher: 1.0
		// PluginA: 1.0 -> Ok
		// PluginB: 0.1 -> Ok
		// PluginC: 1.1 -> Failed
		USHORT unMinMajorVersion;
		USHORT unMinMinorVersion;

		// Used to prevent a plug-in to be loaded twice.
		GUID guid;
	} 
	WEEPLUGININFO, *LPWEEPLUGININFO;

	// May be extended in the future.
	typedef struct _NOTIFYMESSAGE
	{
		INT32 iMessage;
		LPVOID lpData;
		INT32 iRetCode;
	}
	NOTIFYMESSAGE, *LPNOTIFYMESSAGE;

//////////////////////////////////////////////////////////////////////////

// Prototypes.
struct DIFFITEM;
struct DIFFGROUP;

/************************************************************************/
/* The DiffPatcher needs those three structs to organize everything properly.
/* Each item can have a reference to a type and a group.
/* The type and the group itself have a internal list to keep tracking of
/* items that are related to them.
/* 
/* The item struct can have hash created from the string itself. It is up to you
/* to decide what kind of function you will use to create one. The hash is used
/* just to speed up search operations based on the item name.
/************************************************************************/

	struct DIFFTYPE
	{
		INT32 iTypeId;
		LPTSTR lpszName;
		std::vector<DIFFITEM *> items;
	};

	struct DIFFGROUP
	{
		INT32 iGroupId;
		LPTSTR lpszName;
		std::vector<DIFFITEM *> items;
	};

	struct DIFFITEM
	{
		LPTSTR lpszName;
		LPTSTR lpszDescription;
		INT32 iHash;
		// You can set this state before GetDiffItems() is called in order to (for example) 
		// set a check box state in the list view before it gets filled.
		UINT32 uState;
		// Never used by the DiffPatcher so don't hesitate to associate all important stuff 
		// with it that are related to the item.
		LPVOID lpData;
		DIFFTYPE *lpType;
		DIFFGROUP *lpGroup;
	};

	// Typedefs are easier to maintain after all. :(
	typedef std::vector<DIFFTYPE *> DiffTypeList;
	typedef std::vector<DIFFGROUP *> DiffGroupList;
	typedef std::vector<DIFFITEM *> DiffItemList;

	typedef struct _DIFFITEMLIST
	{
		DiffTypeList types;
		DiffGroupList groups;
		DiffItemList items;
	}
	DIFFITEMLIST, *LPDIFFITEMLIST;

//////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

typedef HMODULE HPLUGIN;
typedef std::vector<LPTSTR> ColumnNames; // Vector contains a list of column names that will be displayed when the plug-in is being loaded.

// Those are notifications that are sent from the DiffPatcher to inform the plug-in about
// changes or other stuff.
enum {
	
	// v1.0
	NM_EXE_CHANGED,			// Path to input executable has changed. The lpData member contains the LPTSTR.
	NM_DIFF_CHANGED,		// Path to input diff has changed. The lpData member contains the LPTSTR.
	NM_OUTPUT_CHANGED,		// Path to output executable has changed. The lpData member contains the LPTSTR.

	// v1.1
	NM_ITEMSELECTED,		// Notifies that an item has been selected. THe lpData member contains a pointer to the DIFFITEM.
	NM_ITEMDESELECTED,		// Notifies that an item has been deselected. THe lpData member contains a pointer to the DIFFITEM.
	NM_ITEMPRESELECTED,		// Sent to the plug-in whenever an item is going to be checked. Set iRetCode to a non-zero value to prevent checking.
	NM_ITEMPREDESELECTED	// Sent to the plug-in whenever an item is going to be checked. Set iRetCode to a non-zero value to prevent unchecking
};

// Maybe a design mistake.. however, I'm not quite sure if this will change sometime.
enum {
	ST_ENABLED = 1
};

// Error codes that aren't really necessary but may help the DiffPatcher
// to track 'em down in the future.
enum
{
	E_INVALID_EXE = -6,
	E_WRITE_FILE = -5,
	E_MEMORY = -4,
	E_EMPTY = -3,
	E_INVALID_FORMAT = -2,
	E_READ_FILE = -1
};

//////////////////////////////////////////////////////////////////////////
// Interfaces

/************************************************************************/
/* The plug-in will receive a pointer to a class that implements
/* this interface and can therefore use it to call functions provided by
/* the DiffPatcher itself.
/************************************************************************/
class IGUI
{
public:

	// Displays.. a message box. It works almost the same as the default windows message box except it draws
	// a check box at the bottom of the dialog and who cares, it uses an owner drawn windows vista styled dialog box.
	// To enable the check box you have to add MB_CHECKBOX to the style parameter.
	// Use a combination of MB_TYPEMASK and IDCHECKBOX to see if check box was active and what message was returned.
	virtual INT32 DisplayMessageBox(LPCTSTR lpszCaption, LPCTSTR lpszText, LPCTSTR lpszCheckbox, UINT32 uIcondIndex, INT32 iStyle) = 0;

	/************************************************************************/
	/* The DiffPatcher was supposed to give as much freedom as possible.
	/* It may be dangerous to offer such function, however, it's still worth it.
	/************************************************************************/
	virtual HWND GetMainHandle() = 0;
};

/************************************************************************/
/* This is the interface your plug-in must implement in order
/* to be marked as valid DiffPatcher plug-in.
/* Don't forget to set the IGUI parameter when you deriver from this class
/* or you won't be able to call DiffPatchers functions!
/************************************************************************/

/************************************************************************/
/* Version 1.0
/************************************************************************/
class IPlugin
{
public:
	// Save the GUI pointer.
	IPlugin(IGUI *gui)
	{
		m_gui = gui;
	}

	// Called when the plug-in is beeing unloaded.
	// This is mostly the case when the user closes the DiffPatcher.
	virtual void Release() = 0;

	// As you may already noticed by yourself,
	// each plug-in has his own memory manager (Multi-threaded runtime library /MT) and therefore
	// the DiffPatcher cannot freely release memory allocated by your plug-in.
	// So provide at least a mechanism to release memory.
	virtual void Free(LPVOID memory) = 0;

	// You don't have to, but you can (of course) provide a dialog box to display
	// information about the plug-in.
	virtual void About(HWND hParent) = 0;

	// Self-explanatory.
	virtual LPWEEPLUGININFO GetPluginInfo() = 0;

	// Return a pointer to a string that contains 1(!) diff extension supported
	// by your plug-in. E.g. ".diff", or ".xdiff", or ".awesome", etc.
	// When the user clicks on the Open DiffFile Button, then the filter
	// will be automatically adjusted to include this supported file format.
	// Example:
	// PluginA: .diff
	// PluginB: .xdiff
	// OpenFileName Filter: Supported diff files (*.diff; *.xdiff)\0*.diff;*.xdiff;\0\0
	virtual LPCTSTR GetSupportedFormat() = 0;

	// Return a pointer to a string that will be drawn at the top of the DiffPatcher.
	virtual LPCTSTR GetDiffTitle() = 0;

	// Your plug-in will receive all notification messages through this
	// function in form of a NOTIFYMESSAGE struct.
	virtual void Notify(LPNOTIFYMESSAGE sMessage) = 0;

	// If your plug-in doesn't need a diff file at all (which I will indeed use in the next plug-in),
	// you can return false in order to disable the button that will open a dialog box for the
	// input diff file.
	virtual bool NeedDiffFile() = 0;

	// Self-explanatory.
	virtual INT32 PatchIt() = 0;

	// Returns the count of all diffs. Yeah, I know, design failure, since the DiffPatcher can call
	// the size() function of the std::vector returned by the function below. :(
	virtual INT32 GetDiffCount() = 0;

	// Returns a pointer to the DIFFITEMLIST structure.
	virtual LPDIFFITEMLIST GetDiffItems() = 0;

	// Self-explanatory.
	IGUI *m_gui;
};

/************************************************************************/
/* Version 1.1
/************************************************************************/
class IPlugin2
{
public:
	// Allows you to skip the check that prevents selection if diffs that are in the same group.
	virtual bool PreventGroupCollision() = 0;

	// Returns a pointer to a vector which contains a list of column names.
	virtual ColumnNames *GetColumnNames() = 0;

	// Will be passed from the main loop to ensure that the plug-in is able to return own display names.
	virtual void GetColumnDispInfo(WeePlugin::DIFFITEM *lpDiffItem, NMLVDISPINFO *lpDispInfo) = 0;
};

// Each plug-in must export the InitPlugin function.
// I don't care what kind of initialization you will use
// as long as you return a pointer to the class that derivered
// from the IPlugin interface.
typedef IPlugin *(*fnInitPlugin)(IGUI *, USHORT, USHORT);

}

#endif // _WEE_DIFF_PLUGIN_H