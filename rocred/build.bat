@ECHO OFF
REM ----------------------------------------------------------------
REM RO Credentials (ROCred)
REM (c) 2012-2013 Ai4rei/AN
REM
REM ----------------------------------------------------------------

:CLEAN
FOR %%v IN (*.obj snippets.lib rocred.res) DO IF EXIST %%v DEL %%v
IF "%1"=="clean" GOTO END
:BUILD
SET CLOPT=/nologo /c /W3 /O1s /GF /GA /IC:\Progra~1\Micros~1\SDK\include /I..\snippets
FOR %%v IN (dlgabout.c dlgtempl.c kvdb.c kvdb\win32ini.c macaddr.c md5.c memory.c regionui.c xf_binhex.c) DO CL %CLOPT% ..\snippets\%%v
LIB /NOLOGO /OUT:snippets.lib *.obj
DEL *.obj > NUL
RC /Forocred.res rocred.rc
CL %CLOPT% rocred.c bgskin.c config.c
LINK /NOLOGO /OPT:REF /OPT:ICF /OPT:NOWIN98 /RELEASE /OUT:rocred.exe *.obj rocred.res snippets.lib kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib
SET CLOPT=
:END
