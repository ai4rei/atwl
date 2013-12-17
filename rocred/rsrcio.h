#ifndef _RSRCIO_H_
#define _RSRCIO_H_

static inline bool __stdcall ResourceFetch(HINSTANCE hInstance, const char* lpszType, const char* lpszName, const void** lppOut, unsigned long* lpluSize)
{
    HRSRC hInfo = FindResourceA(hInstance, lpszName, lpszType);

    if(hInfo)
    {
        HGLOBAL hConf = LoadResource(hInstance, hInfo);

        if(hConf)
        {
            void* lpRsrc = LockResource(hConf);

            if(lpRsrc)
            {
                lppOut[0]   = lpRsrc;
                lpluSize[0] = SizeofResource(hInstance, hInfo);

                return true;
            }
        }
    }

    return false;

    (void)&ResourceFetch;
}

static inline bool __stdcall ResourceStore(const char* lpszFileName, const char* lpszType, const char* lpszName, const void* lpIn, unsigned long luSize)
{
    HANDLE hUpdate = BeginUpdateResource(lpszFileName, FALSE);

    if(hUpdate)
    {
        if(UpdateResource(hUpdate, lpszType, lpszName, 0, (LPVOID)lpIn, luSize))
        {
            if(EndUpdateResource(hUpdate, FALSE))
            {
                return true;
            }
        }
        else
        {
            EndUpdateResource(hUpdate, TRUE);
        }
    }

    return false;

    (void)&ResourceStore;
}

#endif  /* _RSRCIO_H_ */
