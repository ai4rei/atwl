#include "WeePluginManager.h"

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

WeePluginManager::WeePluginManager() {}

WeePluginManager::~WeePluginManager()
{
	for(PluginMap::const_iterator it = m_plugins.begin(); it != m_plugins.end(); it++)
		UnloadPlugin(it);
	
	m_plugins.clear();
}

INT32 WeePluginManager::LoadPlugin(LPCTSTR lpszPluginPath, LPVOID lpData, USHORT unMajorVersion, USHORT unMinorVersion)
{
	if(lpszPluginPath == 0 || _tcslen(lpszPluginPath) < 4)
		return INVALID_POINTER;

	WeePlugin::HPLUGIN hPlugin = LoadLibrary(lpszPluginPath);
	if(hPlugin == 0)
		return LOADLIBRARY_FAILED;

	WeeDiffGenPlugin::fnInitPlugin fnInitPlugin = (WeeDiffGenPlugin::fnInitPlugin)GetProcAddress(hPlugin, "InitPlugin");
	if(fnInitPlugin == 0)
	{
		FreeLibrary(hPlugin);
		return GETINTERFACE_FAILED;
	}

	WeeDiffGenPlugin::IWDGPlugin *wdp = fnInitPlugin(lpData, unMajorVersion, unMinorVersion);
	if(wdp == 0)
	{
		FreeLibrary(hPlugin);
		return LOADPLUGIN_FAILED;
	}

	WeeDiffGenPlugin::LPWDGPLUGININFO lp_wpi = wdp->GetPluginInfo();
	if(lp_wpi == 0)
	{
		FreeLibrary(hPlugin);
		return LOADPLUGIN_FAILED;
	}

	if((lp_wpi->unMinMajorVersion == unMajorVersion && lp_wpi->unMinMinorVersion > unMinorVersion) ||
		(lp_wpi->unMinMajorVersion > unMajorVersion))
	{
		FreeLibrary(hPlugin);
		return INCOMPATIBLE_VESION;
	}

	for(PluginMap::const_iterator it = m_plugins.begin(); it != m_plugins.end(); it++)
	{
		if(memcmp(&it->second.guid, &lp_wpi->guid,sizeof(GUID)) == 0)
		{
			FreeLibrary(hPlugin);
			return ALREADY_LOADED;
		}
	}

	PLUGIN sPlugin;
	sPlugin.guid = lp_wpi->guid;
	sPlugin.lpInterface = wdp;

	m_plugins.insert(std::make_pair(hPlugin, sPlugin));

	return SUCCESS;
}

INT32 WeePluginManager::UnloadPlugin(WeePlugin::HPLUGIN hPlugin)
{
	PluginMap::iterator it = m_plugins.find(hPlugin);

	if(it != m_plugins.end())
	{
		WeeDiffGenPlugin::IWDGPlugin *wdp = it->second.lpInterface;

		if(wdp != NULL)
		{
			wdp->Release();
			wdp = NULL;
		}

		return SUCCESS;
	}

	return PLUGIN_NOT_FOUND;
}

INT32 WeePluginManager::UnloadPlugin(PluginMap::const_iterator &it)
{
	if(it != m_plugins.end())
	{
		WeeDiffGenPlugin::IWDGPlugin *wdp = it->second.lpInterface;

		if(wdp != NULL)
		{
			wdp->Release();
			wdp = NULL;
		}

		return SUCCESS;
	}

	return PLUGIN_NOT_FOUND;
}