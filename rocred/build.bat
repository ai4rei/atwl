@ECHO OFF
REM ----------------------------------------------------------------
REM RO Credentials (ROCred)
REM (c) 2012-2014 Ai4rei/AN
REM
REM ----------------------------------------------------------------

:CLEAN
FOR %%v IN (*.obj snippets.lib rocred.res) DO IF EXIST %%v DEL %%v
IF "%1"=="clean" GOTO END
:BUILD
SET CLOPT=/nologo /c /W3 /O1s /GF /GA /IC:\Progra~1\Micros~1\SDK\include /I..\snippets
FOR %%v IN (bvector.c bvcstr.cpp bvfont.cpp bvpars.cpp dlgabout.c dlgtempl.c kvdb.c kvdb\win32ini.c macaddr.c md5.c memory.c procbth.c regionui.c w32ui.cpp w32uxt.c wnttools.c xf_binhex.c xf_slash.c) DO CL %CLOPT% ..\snippets\%%v
LIB /NOLOGO /OUT:snippets.lib *.obj
DEL *.obj > NUL
RC /Forocred.res /I..\snippets rocred.rc
CL %CLOPT% rocred.c bgskin.c button.c config.c
LINK /NOLOGO /OPT:REF /OPT:ICF /OPT:NOWIN98 /RELEASE /DEBUG /OUT:rocred.exe *.obj rocred.res snippets.lib kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib advapi32.lib ole32.lib
MODPE /NOLOGO /RELEASE /DLLFLAGS +NXCOMPAT rocred.exe
SET CLOPT=
:END
