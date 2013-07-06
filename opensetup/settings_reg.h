// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _SETTINGS_REG_H_
#define _SETTINGS_REG_H_

class CSettingsReg : public CSettings
{
public:
    bool __stdcall Save(void);
    void __stdcall Load(void);
    void __stdcall Reset(void);
    void __stdcall ResetSettings(void);
    bool __stdcall IsAvail(SETTINGENTRY nEntry);
    bool __stdcall IsAdminRequired(void);
    SETTINGENGINEID __stdcall GetEngineID(void);
};

#endif  /* _SETTINGS_REG_H_ */
