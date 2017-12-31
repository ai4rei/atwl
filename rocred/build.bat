@ECHO OFF
REM ----------------------------------------------------------------
REM RO Credentials (ROCred)
REM (c) 2012+ Ai4rei/AN
REM
REM ----------------------------------------------------------------

:CLEAN
FOR %%v IN (*.obj snippets.lib rocred.res) DO IF EXIST %%v DEL %%v
IF "%1"=="clean" GOTO END
:BUILD
SET CLOPT=/nologo /c /W3 /O2x /GF /GA /DBVLLST_CHECK_UNBOUND_INSERT /Zi /I..\snippets
FOR %%v IN (bvllst dlgabout dlgtempl kvdb _kvdb\win32ini macaddr md5 mem _mem\mem.win32.heap regionui rsrcio w32uxt wnttools xf_slash) DO CL %CLOPT% ..\snippets\%%v.c
FOR %%v IN (bvcstr bvfont bvpars bvwide w32ex w32ui xf_binhex) DO CL %CLOPT% ..\snippets\%%v.cpp
LIB /NOLOGO /OUT:snippets.lib *.obj
DEL *.obj > NUL
RC /Forocred.res /I..\snippets rocred.rc
CL %CLOPT% rocred.c bgskin.c button.c config.c
LINK /NOLOGO /OPT:REF /OPT:ICF /RELEASE /DEBUG /LARGEADDRESSAWARE /FIXED:NO /VERSION:1.9 /OUT:rocred.exe *.obj rocred.res snippets.lib kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib advapi32.lib ole32.lib
MODPE /NOLOGO /RELEASE /DLLFLAGS +DYNAMICBASE+NXCOMPAT+TSAWARE rocred.exe
SET CLOPT=
:END
