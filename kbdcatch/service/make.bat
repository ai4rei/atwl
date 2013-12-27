@ECHO OFF
IF EXIST *.obj DEL *.obj
IF "%1"=="clean" GOTO END
SET CLOPT=/O2x /GF /DNDEBUG
SET LNOPT=/OPT:REF /OPT:ICF /OPT:NOWIN98
IF "%1"=="debug" SET CLOPT=/Od /DDBG /DDEBUG /D_DEBUG
IF "%1"=="debug" SET LNOPT=/OPT:NOREF /OPT:NOICF
FOR %%i IN (bvargs bvdebug cstr memory) DO CL /nologo /c /W3 %CLOPT% /DWINDOWS /D_MBCS ..\..\..\snippets\%%i.c
CL /nologo /c /W3 %CLOPT% /DWINDOWS /D_MBCS /I..\..\..\snippets kbdcserv.c
LINK /NOLOGO %LNOPT% /RELEASE /SUBSYSTEM:WINDOWS /ENTRY:KbdcServEnter /OUT:KBDCSERV.EXE *.obj kernel32.lib user32.lib advapi32.lib
:END
