// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
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

// types
typedef HRESULT (WINAPI *DirectDrawCreateExFunc)(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);
typedef HRESULT (WINAPI *DirectDrawEnumerateExFunc)(LPDDENUMCALLBACKEX lpCallback, LPVOID lpContext, DWORD  dwFlags);

// funcs
static DirectDrawCreateExFunc DX7E_DirectDrawCreateEx;
static DirectDrawEnumerateExFunc DX7E_DirectDrawEnumerateEx;

#ifndef DX7E_DYNAMIC
    #pragma comment(lib, "ddraw.lib")
#endif
#pragma comment(lib, "dxguid.lib")

static HRESULT CALLBACK DX7E_P_EnumDeviceCallback(char* lpDeviceDescription, char* lpDeviceName, D3DDEVICEDESC7* lpD3DDeviceDesc, void* lpContext)
{
    struct DDrawDriverEntry* lpDE = (struct DDrawDriverEntry*)lpContext;
    struct DDrawDeviceEntry* lpEntry;

    lpEntry = &lpDE->Devices[lpDE->luItems];

    if(lpD3DDeviceDesc->dwDevCaps&D3DDEVCAPS_HWRASTERIZATION)
    {
        CopyMemory(&lpEntry->DeviceGuid, &lpD3DDeviceDesc->deviceGUID, sizeof(lpEntry->DeviceGuid));
        lstrcpynA(lpEntry->szName, lpDeviceName, sizeof(lpEntry->szName));

        lpDE->luItems++;
    }

    return (lpDE->luItems>=MAX_DX7DEVICE) ? D3DENUMRET_CANCEL : D3DENUMRET_OK;
}

static HRESULT CALLBACK DX7E_P_EnumDisplayModesCallback(DDSURFACEDESC2* lpDDSurfaceDesc, void* lpContext)
{
    struct DDrawDriverEntry* lpDE = (struct DDrawDriverEntry*)lpContext;
    struct DDrawModeEntry* lpEntry;

    lpEntry = &lpDE->Modes[lpDE->luModes];

    if(lpDDSurfaceDesc->dwSize==sizeof(DDSURFACEDESC2) && (lpDDSurfaceDesc->dwFlags&(DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT)))
    {
        if(lpDDSurfaceDesc->ddpfPixelFormat.dwSize==sizeof(DDPIXELFORMAT) && (lpDDSurfaceDesc->ddpfPixelFormat.dwFlags&DDPF_RGB))
        {
            lpEntry->luWidth    = lpDDSurfaceDesc->dwWidth;
            lpEntry->luHeight   = lpDDSurfaceDesc->dwHeight;
            lpEntry->luBitDepth = lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
            lpDE->luModes++;
        }
    }

    return (lpDE->luModes>=MAX_DISPMODES) ? DDENUMRET_CANCEL : DDENUMRET_OK;
}

static BOOL CALLBACK DX7E_P_EnumDriverCallback(GUID* lpGUID, char* lpDriverDescription, char* lpDriverName, void* lpContext, HMONITOR hMon)
{
    struct DDrawDriverDeviceInfo* lpDI = (struct DDrawDriverDeviceInfo*)lpContext;
    struct DDrawDriverEntry* lpEntry;
    IDirectDraw7* lpDD;
    HRESULT hr;

    lpEntry = &lpDI->Drivers[lpDI->luItems];

    if(lpGUID)
    {
        CopyMemory(&lpEntry->DriverGuid, lpGUID, sizeof(lpEntry->DriverGuid));
    }
    lstrcpynA(lpEntry->szDescription, lpDriverDescription, sizeof(lpEntry->szDescription));

    hr = DX7E_DirectDrawCreateEx(lpGUID, (void**)&lpDD, IID_IDirectDraw7, NULL);

    if(!FAILED(hr))
    {
        lpEntry->DDDriverCaps.dwSize = sizeof(lpEntry->DDDriverCaps);

        if(!FAILED(DX7E_P_GetCaps(lpDD, &lpEntry->DDDriverCaps, NULL)) && !FAILED(DX7E_P_EnumDisplayModes(lpDD, 0, NULL, (void*)lpEntry, &DX7E_P_EnumDisplayModesCallback)))
        {
            IDirect3D7* lpD3D;

            hr = DX7E_P_QueryInterface(lpDD, IID_IDirect3D7, (void**)&lpD3D);

            if(!FAILED(hr))
            {
                DX7E_P_EnumDevices(lpD3D, &DX7E_P_EnumDeviceCallback, (void*)lpEntry);

                if(lpEntry->luItems)
                {
                    lpDI->luItems++;
                }

                DX7E_P_Release(lpD3D);
            }
        }
        DX7E_P_Release(lpDD);
    }

    return (lpDI->luItems>=MAX_DX7DRIVER) ? FALSE : TRUE;
}

bool __stdcall DX7E_EnumDriverDevices(struct DDrawDriverDeviceInfo* lpDI)
{
    HRESULT hr;
#ifdef DX7E_DYNAMIC
    HINSTANCE hDll;

    if((hDll = LoadLibrary("ddraw.dll"))==NULL)
    {
        return false;
    }

    DX7E_DirectDrawCreateEx    = (DirectDrawCreateExFunc)GetProcAddress(hDll, "DirectDrawCreateEx");
    DX7E_DirectDrawEnumerateEx = (DirectDrawEnumerateExFunc)GetProcAddress(hDll, "DirectDrawEnumerateExA");

    if(!DX7E_DirectDrawCreateEx || !DX7E_DirectDrawEnumerateEx)
    {
        FreeLibrary(hDll);
        return false;
    }
#else
    DX7E_DirectDrawCreateEx    = DirectDrawCreateEx;
    DX7E_DirectDrawEnumerateEx = DirectDrawEnumerateEx;
#endif

    hr = DX7E_DirectDrawEnumerateEx(&DX7E_P_EnumDriverCallback, (void*)lpDI, DDENUM_ATTACHEDSECONDARYDEVICES|DDENUM_DETACHEDSECONDARYDEVICES|DDENUM_NONDISPLAYDEVICES);

#ifdef DX7E_DYNAMIC
    FreeLibrary(hDll);
#endif

    if(FAILED(hr))
    {
        return false;
    }
    return true;
}
