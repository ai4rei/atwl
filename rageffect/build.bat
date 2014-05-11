@ECHO OFF
IF EXIST *.obj DEL *.obj
IF EXIST *.res DEL *.res
RC /Forageffect.res /IE:\_dev\snippets rageffect.rc
FOR %%i IN (bvdebug.c bvpe.c cstr.c hashdb.c lhash.c memory.c ioapix.win.c ragtok.c) DO CL /nologo /c /W3 /O2x /GF /MT /DBTYPES_NO__ARRAYSIZE E:\_dev\snippets\%%i
CL /nologo /c /W3 /O2x /GF /MT /IE:\_dev\snippets /DBTYPES_NO__ARRAYSIZE rageffect.c
LINK /NOLOGO /DLL /RELEASE /DEBUG /MACHINE:IX86 /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /OPT:NOWIN98 /OUT:rageffect.asi user32.lib *.obj rageffect.res
