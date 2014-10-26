// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

// ddraw.dll
// > DirectDrawCreateEx
// > DirectDrawEnumerateExA

#define DIRECT3D_VERSION 0x0700
#define DIRECTDRAW_VERSION 0x0700

#include <windows.h>
#include <d3d.h>
#include <d3dcaps.h>
#include <ddraw.h>

#include "dx7enum.h"

#ifndef DX7E_DYNAMIC
    #pragma comment(lib, "ddraw.lib")
#endif
#pragma comment(lib, "dxguid.lib")

#ifndef __ARRAYSIZE
    #ifndef _ARRAYSIZE
        #define __ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
    #else  /* _ARRAYSIZE */
        #define __ARRAYSIZE _ARRAYSIZE
    #endif  /* _ARRAYSIZE */
#endif  /* __ARRAYSIZE */

/**
 * @brief   Prototypes for ddraw.dll functions.
 ******************************************************************/
typedef HRESULT (WINAPI* LPFNDIRECTDRAWCREATEEX)(
    GUID FAR* lpGUID,
    LPVOID* lplpDD,
    REFIID iid,
    IUnknown FAR* pUnkOuter
);
typedef HRESULT (WINAPI* LPFNDIRECTDRAWENUMERATEEX)(
    LPDDENUMCALLBACKEX lpCallback,
    LPVOID lpContext,
    DWORD dwFlags
);

/**
 * @brief   Structure for internal data passing.
 ******************************************************************/
typedef struct DX7ESTATE
{
    LPDX7EDISPLAYDRIVERINFO lpDDI;
    LPFNDIRECTDRAWCREATEEX DirectDrawCreateEx;
    LPFNDIRECTDRAWENUMERATEEX DirectDrawEnumerateEx;
#ifdef DX7E_DYNAMIC
    HMODULE hDll;
#endif  /* DX7E_DYNAMIC */
}
DX7ESTATE,* LPDX7ESTATE;
typedef const DX7ESTATE* LPCDX7ESTATE;

static HRESULT CALLBACK DX7E_P_EnumDevicesForEach(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpD3DDeviceDesc, LPVOID lpContext)
{
    LPDX7EDISPLAYDRIVER lpDriver = (LPDX7EDISPLAYDRIVER)lpContext;
    LPDX7ERENDERDEVICE lpDevice;

    if(lpD3DDeviceDesc->dwDevCaps&D3DDEVCAPS_HWRASTERIZATION)
    {
        if(lpDriver->luDevicesHW>=__ARRAYSIZE(lpDriver->DeviceHW))
        {// try next
            return DDENUMRET_OK;
        }

        lpDevice = &lpDriver->DeviceHW[lpDriver->luDevicesHW++];
    }
    else
    {
        if(lpDriver->luDevicesSW>=__ARRAYSIZE(lpDriver->DeviceSW))
        {// try next
            return DDENUMRET_OK;
        }

        lpDevice = &lpDriver->DeviceSW[lpDriver->luDevicesSW++];
    }

    if(DX7E_P_IsEqualIID(lpD3DDeviceDesc->deviceGUID, IID_IDirect3DRGBDevice))
    {
        lpDevice->nType = DX7E_RDT_HEL;
    }
    else
    if(DX7E_P_IsEqualIID(lpD3DDeviceDesc->deviceGUID, IID_IDirect3DHALDevice))
    {
        lpDevice->nType = DX7E_RDT_HAL;
    }
    else
    if(DX7E_P_IsEqualIID(lpD3DDeviceDesc->deviceGUID, IID_IDirect3DTnLHalDevice))
    {
        lpDevice->nType = DX7E_RDT_HAL_TNL;
    }
    else
    {
        lpDevice->nType = DX7E_RDT_UNKNOWN;
    }

    lstrcpynA(lpDevice->szName, lpDeviceName, __ARRAYSIZE(lpDevice->szName));

    return DDENUMRET_OK;
}

static HRESULT CALLBACK DX7E_P_EnumDisplayModesForEach(LPDDSURFACEDESC2 lpDDSD, LPVOID lpContext)
{
    LPDX7EDISPLAYDRIVER lpDriver = (LPDX7EDISPLAYDRIVER)lpContext;

    if(lpDDSD->dwSize==sizeof(DDSURFACEDESC2) && (lpDDSD->dwFlags&(DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT)))
    {
        if(lpDDSD->ddpfPixelFormat.dwSize==sizeof(DDPIXELFORMAT) && (lpDDSD->ddpfPixelFormat.dwFlags&DDPF_RGB))
        {
            LPDX7EDISPLAYMODE lpMode = &lpDriver->Mode[lpDriver->luModes++];

            lpMode->luWidth    = lpDDSD->dwWidth;
            lpMode->luHeight   = lpDDSD->dwHeight;
            lpMode->luBitDepth = lpDDSD->ddpfPixelFormat.dwRGBBitCount;
        }
    }

    return lpDriver->luModes>=__ARRAYSIZE(lpDriver->Mode) ? DDENUMRET_CANCEL : DDENUMRET_OK;
}

static BOOL CALLBACK DX7E_P_DirectDrawEnumerateForEach(GUID FAR* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hMon)
{
    HRESULT hr;
    LPDX7ESTATE S = (LPDX7ESTATE)lpContext;
    IDirectDraw7* lpDD;

    hr = DX7E_DirectDrawCreateEx(S, lpGUID, (LPVOID*)&lpDD, IID_IDirectDraw7, NULL);

    if(FAILED(hr))
    {
        ;
    }
    else
    {
        LPDX7EDISPLAYDRIVER lpDriver = &S->lpDDI->Driver[S->lpDDI->luDrivers];

        // initialize
        lpDriver->luModes = 0;

        // enumerate
        hr = DX7E_P_EnumDisplayModes(lpDD, 0, NULL, (LPVOID)lpDriver, &DX7E_P_EnumDisplayModesForEach);

        if(FAILED(hr))
        {
            ;
        }
        else
        {
            IDirect3D7* lpD3D;

            hr = DX7E_P_QueryInterface(lpDD, IID_IDirect3D7, (LPVOID*)&lpD3D);

            if(FAILED(hr))
            {
                ;
            }
            else
            {
                // initialize
                lpDriver->luDevicesHW = lpDriver->luDevicesSW = 0;

                // enumerate
                hr = DX7E_P_EnumDevices(lpD3D, &DX7E_P_EnumDevicesForEach, (LPVOID)lpDriver);

                if(FAILED(hr))
                {
                    ;
                }
                else if(lpDriver->luDevicesHW || lpDriver->luDevicesSW)
                {
                    if(lpGUID)
                    {
                        CopyMemory(&lpDriver->DriverGuid, lpGUID, sizeof(lpDriver->DriverGuid));
                    }
                    else
                    {
                        ZeroMemory(&lpDriver->DriverGuid, sizeof(lpDriver->DriverGuid));
                    }
                    lstrcpynA(lpDriver->szName, lpDriverDescription, __ARRAYSIZE(lpDriver->szName));
                    S->lpDDI->luDrivers++;
                }

                DX7E_P_Release(lpD3D);
            }
        }

        DX7E_P_Release(lpDD);
    }

    return S->lpDDI->luDrivers>=__ARRAYSIZE(S->lpDDI->Driver) ? FALSE : TRUE;
}

bool __stdcall DX7E_EnumDriverDevices(LPDX7EDISPLAYDRIVERINFO lpDDI)
{
    DX7ESTATE S = { lpDDI };
    HRESULT hr;

#ifdef DX7E_DYNAMIC
    S.hDll = LoadLibrary("ddraw.dll");

    if(!S.hDll)
    {
        return false;
    }

    S.DirectDrawCreateEx    = (LPFNDIRECTDRAWCREATEEX)GetProcAddress(S.hDll, "DirectDrawCreateEx");
    S.DirectDrawEnumerateEx = (LPFNDIRECTDRAWENUMERATEEX)GetProcAddress(S.hDll, "DirectDrawEnumerateExA");

    if(!S.DirectDrawCreateEx || !S.DirectDrawEnumerateEx)
    {
        FreeLibrary(S.hDll);
        return false;
    }
#else  /* DX7E_DYNAMIC */
    S.DirectDrawCreateEx    = &DirectDrawCreateEx;
    S.DirectDrawEnumerateEx = &DirectDrawEnumerateExA;
#endif  /* DX7E_DYNAMIC */

    // initialize
    lpDDI->luDrivers = 0;

    // enumerate
    hr = DX7E_DirectDrawEnumerateEx(&S, &DX7E_P_DirectDrawEnumerateForEach, (LPVOID)&S, DDENUM_ATTACHEDSECONDARYDEVICES|DDENUM_DETACHEDSECONDARYDEVICES|DDENUM_NONDISPLAYDEVICES);

#ifdef DX7E_DYNAMIC
    FreeLibrary(S.hDll);
#endif  /* DX7E_DYNAMIC */

    if(FAILED(hr))
    {
        return false;
    }

    return true;
}

const GUID* __stdcall DX7E_DeviceType2Guid(DX7ERENDERDEVICETYPE nType)
{
    switch(nType)
    {
        case DX7E_RDT_HEL:      return &IID_IDirect3DRGBDevice;
        case DX7E_RDT_HAL:      return &IID_IDirect3DHALDevice;
        case DX7E_RDT_HAL_TNL:  return &IID_IDirect3DTnLHalDevice;
    }

    return NULL;
}
