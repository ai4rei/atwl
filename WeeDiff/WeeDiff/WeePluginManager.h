#ifndef _WEE_PLUGIN_MANAGER_H
#define _WEE_PLUGIN_MANAGER_H

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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <tchar.h>
#include <map>
#include <iterator>

#include <Snippets/WeeException.h>
#include <Common/WeeDiffPlugin.h>
#include "WeeGUInterface.h"

class WeePluginManager
{
public:

	class IBasePlugin : public WeePlugin::IPlugin, public WeePlugin::IPlugin2 {};

	WeePluginManager();
	~WeePluginManager();

	typedef enum _WDPM_RET_CODE
	{
		ALREADY_LOADED = -7,
		PLUGIN_NOT_FOUND = -6,
		INCOMPATIBLE_VESION = -5,
		LOADPLUGIN_FAILED = -4,
		GETINTERFACE_FAILED = -3,
		LOADLIBRARY_FAILED = -2,
		INVALID_POINTER = -1,
		SUCCESS = 0,
	} WDPM_RET_CODE;

	typedef struct PLUGIN
	{
		GUID guid;
		IBasePlugin *lpInterface;
	} PLUGIN, *LPPLUGIN;

	typedef std::map<WeePlugin::HPLUGIN, PLUGIN> PluginMap;

	INT32 LoadPlugin(LPCTSTR lpszPluginPath, WeeGUI *gui, USHORT unMajorVersion, USHORT unMinorVersion);
	INT32 UnloadPlugin(WeePlugin::HPLUGIN);
	INT32 UnloadPlugin(PluginMap::const_iterator &it);

	PluginMap m_plugins;
};

#endif // _WEE_PLUGIN_MANAGER_H