@ECHO OFF
IF EXIST *.res DEL *.res
IF EXIST *.obj DEL *.obj
IF "%1"=="clean" GOTO END
SET CLDEF=/DNDEBUG
SET CLOPT=/O2x /GF %CLDEF%
SET LNOPT=/OPT:REF /OPT:ICF /OPT:NOWIN98
IF "%1"=="debug" SET CLDEF=/DDBG /DDEBUG /D_DEBUG
IF "%1"=="debug" SET CLOPT=/Od %CLDEF%
IF "%1"=="debug" SET LNOPT=/OPT:NOREF /OPT:NOICF
RC %CLDEF% /I..\..\..\snippets kbdcserv.rc
FOR %%i IN (bvargs bvdebug cstr memory) DO CL /nologo /c /W3 %CLOPT% /DWINDOWS /D_MBCS ..\..\..\snippets\%%i.c
CL /nologo /c /W3 %CLOPT% /D_MBCS /I..\..\..\snippets kbdcserv.c
LINK /NOLOGO %LNOPT% /RELEASE /SUBSYSTEM:CONSOLE /ENTRY:KbdcServEnter /OUT:KBDCSERV.EXE *.obj kbdcserv.res kernel32.lib user32.lib advapi32.lib
:END
SET LNOPT=
SET CLOPT=
SET CLDEF=
