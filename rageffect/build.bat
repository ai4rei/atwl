@ECHO OFF
IF EXIST *.obj DEL *.obj
IF EXIST *.res DEL *.res
RC /Forageffect.res /IE:\_dev\snippets rageffect.rc
IF ERRORLEVEL 1 GOTO NOTOK
FOR %%i IN (bvcstr.cpp w32ex.cpp bvdebug.c bvpe.c hashdb.c lhash.c mem.c _mem\mem.win32.heap.c memory.c ioapix.win.c ragtok.c) DO CL /nologo /c /W3 /O2x /GF /MT /Zi E:\_dev\snippets\%%i
CL /nologo /c /W3 /O2x /GF /MT /IE:\_dev\snippets /Zi rageffect.c
IF ERRORLEVEL 1 GOTO NOTOK
LINK /NOLOGO /DLL /RELEASE /DEBUG /LARGEADDRESSAWARE /FIXED:NO /MACHINE:IX86 /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /OUT:rageffect.asi user32.lib *.obj rageffect.res
IF ERRORLEVEL 1 GOTO NOTOK
MODPE /NOLOGO /RELEASE /DLLFLAGS +DYNAMICBASE+NXCOMPAT rageffect.asi
IF ERRORLEVEL 1 GOTO NOTOK
E:\_dev\fastsign\FASTSIGN -i rageffect.asi
:NOTOK
