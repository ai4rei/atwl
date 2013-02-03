// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _SETTINGS_LUA_H_
#define _SETTINGS_LUA_H_

class CSettingsLua : public CSettings
{
public:
    void __stdcall Save(void);
    void __stdcall Load(void);
    void __stdcall Reset(void);
    bool __stdcall IsAvail(SETTINGENTRY nEntry);
    bool __stdcall IsAdminRequired(void);
    SETTINGENGINEID __stdcall GetEngineID(void);
};

#endif  /* _SETTINGS_LUA_H_ */
