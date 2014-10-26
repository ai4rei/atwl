// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2014 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _SETTINGS_REG_H_
#define _SETTINGS_REG_H_

class CSettingsReg : public CSettings
{
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

#endif  /* _SETTINGS_REG_H_ */
