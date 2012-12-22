// -----------------------------------------------------------------
// NOTE: this is trimmed-down version with dependencies only
// Must not be used with other software than RO Open Setup.
// (c) 2009-2010 Ai4rei/AN
// -----------------------------------------------------------------

#ifndef _CPENUM_H_
#define _CPENUM_H_

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

enum
{
    MAX_CODEPAGE_ENTRIES = 50,
    MAX_CODEPAGE_NAMELEN = 260,
    //
    CPE_FLAG_INSTALLEDONLY = 0x1,
    CPE_FLAG_SORTID        = 0x2,
};

struct CodePageEnumInfoEntry
{
    unsigned long luCodePage;
    char szCodePageName[MAX_CODEPAGE_NAMELEN];
};

struct CodePageEnumInfo
{
    unsigned long luEntries;
    struct CodePageEnumInfoEntry Entries[MAX_CODEPAGE_ENTRIES];
};

bool __stdcall CPE_EnumCodePagesEx(struct CodePageEnumInfo* lpCpe, int nOptions);
void __stdcall CPE_Init(void);
void __stdcall CPE_Quit(void);

#ifdef __cplusplus
};
#endif  /* __cplusplus */

#endif  /* _CPENUM_H_ */
