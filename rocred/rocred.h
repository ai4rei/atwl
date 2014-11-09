// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012-2014 Ai4rei/AN
//
// -----------------------------------------------------------------

#ifndef _ROCRED_H_
#define _ROCRED_H_

#define APP_VERSION "1.7.1.0"
#define APP_VERSIONINFO_VERSION 1,7,1,0

#define IDC_USERNAME                    101
#define IDC_PASSWORD                    102
#define IDC_CHECKSAVE                   103
#define IDB_REPLAY                      104
#define IDB_CUSTOM_BASE                 200
#define IDS_USERNAME                    1001
#define IDS_PASSWORD                    1002
#define IDS_CHECKSAVE                   1003
#define IDS_TITLE                       1004
#define IDS_OK                          1005
#define IDS_CLOSE                       1006
#define IDS_USER_NONE                   1007
#define IDS_USER_SHRT                   1008
#define IDS_PASS_NONE                   1009
#define IDS_PASS_SHRT                   1010
#define IDS_EXE_ERROR                   1011
#define IDS_REPLAY                      1012
#define IDS_CONFIG_ERROR                1013
#define IDS_MISCINFO_PROMPT_PREFIX      1014
#define IDS_MISCINFO_PROMPT_SUFFIX      1015
#define IDS_MISCINFO_OPT_MACADDRESS     1016

int __stdcall MsgBox(HWND hWnd, LPSTR lpszText, DWORD dwFlags);

#endif  /* _ROCRED_H_ */
