// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010 Ai4rei/AN
// See doc/license.txt for details.
// -----------------------------------------------------------------

#ifndef _DX7ENUM_H_
#define _DX7ENUM_H_

enum
{
    MAX_DX7DRIVER = 16,
    MAX_DX7DEVICE = 32,
    MAX_DISPMODES = 64,
};

struct DDrawDeviceEntry
{
    char szName[40];
    GUID DeviceGuid;
};

struct DDrawModeEntry
{
    unsigned long luWidth;
    unsigned long luHeight;
    unsigned long luBitDepth;
};

struct DDrawDriverEntry
{
    char szDescription[40];
    DDCAPS DDDriverCaps;
    GUID DriverGuid;
    unsigned long luItems;
    unsigned long luModes;
    struct DDrawDeviceEntry Devices[MAX_DX7DEVICE];
    struct DDrawModeEntry Modes[MAX_DISPMODES];
};

struct DDrawDriverDeviceInfo
{
    unsigned long luItems;
    struct DDrawDriverEntry Drivers[MAX_DX7DRIVER];
};

bool __stdcall DX7E_EnumDriverDevices(struct DDrawDriverDeviceInfo* lpDi);

/* shared private defines */
#ifdef __cplusplus
#define DX7E_P_GetCaps(lpPtr,DDDriverCaps,DDHELCaps)                                      (lpPtr)->GetCaps((DDDriverCaps),(DDHELCaps))
#define DX7E_P_EnumDisplayModes(lpPtr,dwFlags,lpDDSurfaceDesc,lpContext,lpEnumCallback)   (lpPtr)->EnumDisplayModes((dwFlags),(lpDDSurfaceDesc),(lpContext),(lpEnumCallback))
#define DX7E_P_EnumDevices(lpPtr,lpEnumDevicesCallback,lpUserArg)                         (lpPtr)->EnumDevices((lpEnumDevicesCallback),(lpUserArg))
#define DX7E_P_QueryInterface(lpPtr,riid,ppvObj)                                          (lpPtr)->QueryInterface((riid),(ppvObj))
#define DX7E_P_Release(lpPtr)                                                             (lpPtr)->Release()
#else  /* __cplusplus */
#define DX7E_P_GetCaps(lpPtr,DDDriverCaps,DDHELCaps)                                      (lpPtr)->GetCaps((lpPtr),(DDDriverCaps),(DDHELCaps))
#define DX7E_P_EnumDisplayModes(lpPtr,dwFlags,lpDDSurfaceDesc,lpContext,lpEnumCallback)   (lpPtr)->EnumDisplayModes((lpPtr),(dwFlags),(lpDDSurfaceDesc),(lpContext),(lpEnumCallback))
#define DX7E_P_EnumDevices(lpPtr,lpEnumDevicesCallback,lpUserArg)                         (lpPtr)->EnumDevices((lpPtr),(lpEnumDevicesCallback),(lpUserArg))
#define DX7E_P_QueryInterface(lpPtr,riid,ppvObj)                                          (lpPtr)->QueryInterface((lpPtr),(riid),(ppvObj))
#define DX7E_P_Release(lpPtr)                                                             (lpPtr)->Release(lpPtr)
#endif  /* __cplusplus */

#endif  /* _DX7ENUM_H_ */
