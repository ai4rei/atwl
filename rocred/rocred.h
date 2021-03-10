// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012+ Ai4rei/AN
//
// -----------------------------------------------------------------

#ifndef ROCRED_H
#define ROCRED_H

#define APP_VERSION "1.10.0.110"
#define APP_VERSIONINFO_VERSION 1,10,0,110

#define APP_PRODUCTNAME "RO Credentials"
#define APP_DESCRIPTION "Minimalist Client Launcher"
//#define APP_HAS_UI
#define APP_COPYYEAR "2012-2021"
#define APP_INTERNALNAME "ROCred"
#define APP_ORIGINALNAME "rocred.exe"
#define APP_PACKAGENAME "Ai4rei.E.ROCred"

#define IDI_MAINICON                    1
#define IDC_USERNAME                    101
#define IDC_PASSWORD                    102
#define IDC_CHECKSAVE                   103
#define IDB_CUSTOM_BASE                 200
//efine IDS_USERNAME                    1001
//efine IDS_PASSWORD                    1002
#define IDS_CHECKSAVE                   1003
#define IDS_TITLE                       1004
#define IDS_OK                          1005
#define IDS_CLOSE                       1006
#define IDS_USER_NONE                   1007
#define IDS_USER_SHRT                   1008
#define IDS_PASS_NONE                   1009
#define IDS_PASS_SHRT                   1010
#define IDS_EXE_ERROR                   1011
#define IDS_CONFIG_ERROR                1013
#define IDS_MISCINFO_PROMPT_PREFIX      1014
#define IDS_MISCINFO_PROMPT_SUFFIX      1015
#define IDS_MISCINFO_OPT_MACADDRESS     1016
#define IDS_COINIT_ERROR                1017

#define MAX_REGISTRY_KEY_SIZE 255

#define ROCRED_TARGET_NAME "Ai4rei/AN_ROCred_"

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

int __WDECL MsgBox(HWND const hWnd, LPCSTR const lpszTextOrResource, DWORD const dwFlags);
bool __WDECL GetFileClassFromExtension(char const* const lpszExtension, char* const lpszBuffer, size_t const uBufferSize);
bool __WDECL StartClient(HWND hWnd, char const* const lpszExecutable, char const* const lpszParameters);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* ROCRED_H */
