// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _SETTINGS_LUA_H_
#define _SETTINGS_LUA_H_

class CSettingsLua : public CSettings
{
protected:
    const char* __stdcall P_GetLuaPath();
    const char* __stdcall P_GetLuaFile();
    const char* __stdcall P_GetLuaFull();

public:
    bool __stdcall Save();
    void __stdcall Load();
    void __stdcall Reset();
    void __stdcall ResetSettings();
    bool __stdcall IsAvail(SETTINGENTRY nEntry);
    bool __stdcall IsAdminRequired();
    bool __stdcall IsSane();
    SETTINGENGINEID __stdcall GetEngineID();
};

#endif  /* _SETTINGS_LUA_H_ */
