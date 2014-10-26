// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _DX7ENUM_H_
#define _DX7ENUM_H_

/**
 * @brief   Total amount of display drivers to keep. This default
 *          assumes that a 4x4 setup or 3x3 setup with virtual
 *          display drivers (ex. mirrors) is highly unlikely.
 ******************************************************************/
#ifndef DX7E_MAX_DISPLAY_DRIVER
    #define DX7E_MAX_DISPLAY_DRIVER 16U
#endif  /* DX7E_MAX_DISPLAY_DRIVER */

/**
 * @brief   Total amount of hardware render devices. There should
 *          not be more than two (HAL and HAL T&L).
 ******************************************************************/
#ifndef DX7E_MAX_RENDER_DEVICE_HW
    #define DX7E_MAX_RENDER_DEVICE_HW 2U
#endif  /* DX7E_MAX_RENDER_DEVICE_HW */

/**
 * @brief   Total amount of software render devices. There should
 *          not be more than one (HEL).
 ******************************************************************/
#ifndef DX7E_MAX_RENDER_DEVICE_SW
    #define DX7E_MAX_RENDER_DEVICE_SW 1U
#endif  /* DX7E_MAX_RENDER_DEVICE_SW */

/**
 * @brief   Total amount of display modes per display driver.
 ******************************************************************/
#ifndef DX7E_MAX_DISPLAY_MODE
    #define DX7E_MAX_DISPLAY_MODE 512U
#endif  /* DX7E_MAX_DISPLAY_MODE */

/**
 * @brief   Maximum size for a name label.
 ******************************************************************/
#define DX7E_MAX_DISPLAYNAME 40

/**
 * @brief   Enumeration of render device types.
 ******************************************************************/
typedef enum DX7ERENDERDEVICETYPE
{
    DX7E_RDT_HEL = 0,  // IID_IDirect3DRGBDevice
    DX7E_RDT_HAL,      // IID_IDirect3DHALDevice
    DX7E_RDT_HAL_TNL,  // IID_IDirect3DTnLHalDevice
    DX7E_RDT_UNKNOWN,
}
DX7ERENDERDEVICETYPE;

/**
 * @brief   Structure to hold information about a render device. For
 *          both (fast) hardware renderers and (slow) software ones.
 ******************************************************************/
typedef struct DX7ERENDERDEVICE
{
    DX7ERENDERDEVICETYPE nType;
    char szName[DX7E_MAX_DISPLAYNAME];
}
DX7ERENDERDEVICE,* LPDX7ERENDERDEVICE;
typedef const DX7ERENDERDEVICE* LPCDX7ERENDERDEVICE;

/**
 * @brief   Structure to hold information about a display mode.
 ******************************************************************/
typedef struct DX7EDISPLAYMODE
{
    unsigned long luWidth;
    unsigned long luHeight;
    unsigned long luBitDepth;  // 4, 8, 16, 24 or 32
}
DX7EDISPLAYMODE,* LPDX7EDISPLAYMODE;
typedef const DX7EDISPLAYMODE* LPCDX7EDISPLAYMODE;

/**
 * @brief   Structure to hold information about a display driver.
 ******************************************************************/
typedef struct DX7EDISPLAYDRIVER
{
    char szName[DX7E_MAX_DISPLAYNAME];
    unsigned long luModes;
    unsigned long luDevicesHW;
    unsigned long luDevicesSW;
    DX7EDISPLAYMODE Mode[DX7E_MAX_DISPLAY_MODE];
    DX7ERENDERDEVICE DeviceHW[DX7E_MAX_RENDER_DEVICE_HW];
    DX7ERENDERDEVICE DeviceSW[DX7E_MAX_RENDER_DEVICE_SW];
    // DDCAPS Capabilities;
    GUID DriverGuid;
}
DX7EDISPLAYDRIVER,* LPDX7EDISPLAYDRIVER;
typedef const DX7EDISPLAYDRIVER* LPCDX7EDISPLAYDRIVER;

/**
 * @brief   Wrapper structure for DX7E_EnumDisplayDrivers.
 ******************************************************************/
typedef struct DX7EDISPLAYDRIVERINFO
{
    unsigned long luDrivers;
    DX7EDISPLAYDRIVER Driver[DX7E_MAX_DISPLAY_DRIVER];
}
DX7EDISPLAYDRIVERINFO,* LPDX7EDISPLAYDRIVERINFO;
typedef const DX7EDISPLAYDRIVERINFO* LPCDX7EDISPLAYDRIVERINFO;

/**
 * @brief   Enumerates all DirectX 7 accessible display drivers,
 *          their render devices and display modes.
 * @param   lpDDI
 *          Pointer to a DX7EDISPLAYDRIVERINFO structure to hold
 *          enumerated driver information.
 * @return  Whether or not the enumeration was successful.
 ******************************************************************/
bool __stdcall DX7E_EnumDriverDevices(LPDX7EDISPLAYDRIVERINFO lpDDI);

/**
 * @brief   Returns an GUID for respective render device type.
 * @param   nType
 *          One of the DX7ERENDERDEVICETYPE values.
 * @return  Pointer to corresponding device GUID, NULL if type does
 *          does not correspond to any device.
 ******************************************************************/
const GUID* __stdcall DX7E_DeviceType2Guid(DX7ERENDERDEVICETYPE nType);

/* shared private defines */
#ifdef __cplusplus
#define DX7E_DirectDrawCreateEx(S,lpGUID,lplpDD,iid,pUnkOuter)                            (S)->DirectDrawCreateEx((lpGUID),(lplpDD),(iid),(pUnkOuter))
#define DX7E_DirectDrawEnumerateEx(S,lpCallback,lpContext,dwFlags)                        (S)->DirectDrawEnumerateEx((lpCallback),(lpContext),(dwFlags))
#define DX7E_P_GetCaps(lpPtr,DDDriverCaps,DDHELCaps)                                      (lpPtr)->GetCaps((DDDriverCaps),(DDHELCaps))
#define DX7E_P_EnumDisplayModes(lpPtr,dwFlags,lpDDSurfaceDesc,lpContext,lpEnumCallback)   (lpPtr)->EnumDisplayModes((dwFlags),(lpDDSurfaceDesc),(lpContext),(lpEnumCallback))
#define DX7E_P_EnumDevices(lpPtr,lpEnumDevicesCallback,lpUserArg)                         (lpPtr)->EnumDevices((lpEnumDevicesCallback),(lpUserArg))
#define DX7E_P_QueryInterface(lpPtr,riid,ppvObj)                                          (lpPtr)->QueryInterface((riid),(ppvObj))
#define DX7E_P_Release(lpPtr)                                                             (lpPtr)->Release()
#define DX7E_P_IsEqualIID(riid1,riid2)                                                    IsEqualIID((riid1),(riid2))
#else  /* __cplusplus */
#define DX7E_DirectDrawCreateEx(S,lpGUID,lplpDD,iid,pUnkOuter)                            (S)->DirectDrawCreateEx((lpGUID),(lplpDD),&(iid),(pUnkOuter))
#define DX7E_DirectDrawEnumerateEx(S,lpCallback,lpContext,dwFlags)                        (S)->DirectDrawEnumerateEx((lpCallback),(lpContext),(dwFlags))
#define DX7E_P_GetCaps(lpPtr,DDDriverCaps,DDHELCaps)                                      (lpPtr)->lpVtbl->GetCaps((lpPtr),(DDDriverCaps),(DDHELCaps))
#define DX7E_P_EnumDisplayModes(lpPtr,dwFlags,lpDDSurfaceDesc,lpContext,lpEnumCallback)   (lpPtr)->lpVtbl->EnumDisplayModes((lpPtr),(dwFlags),(lpDDSurfaceDesc),(lpContext),(lpEnumCallback))
#define DX7E_P_EnumDevices(lpPtr,lpEnumDevicesCallback,lpUserArg)                         (lpPtr)->lpVtbl->EnumDevices((lpPtr),(lpEnumDevicesCallback),(lpUserArg))
#define DX7E_P_QueryInterface(lpPtr,riid,ppvObj)                                          (lpPtr)->lpVtbl->QueryInterface((lpPtr),&(riid),(ppvObj))
#define DX7E_P_Release(lpPtr)                                                             (lpPtr)->lpVtbl->Release(lpPtr)
#define DX7E_P_IsEqualIID(riid1,riid2)                                                    IsEqualIID(&(riid1),&(riid2))
#endif  /* __cplusplus */

#endif  /* _DX7ENUM_H_ */
