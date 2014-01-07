@ECHO OFF
FOR %%i IN (*.obj *.res) DO IF EXIST %%i DEL %%i
IF "%1"=="clean" GOTO END
SET CLDEF=/DNDEBUG
SET CLOPT=/O2x /GF %CLDEF%
SET LNOPT=/OPT:REF /OPT:ICF /OPT:NOWIN98
IF "%1"=="debug" SET CLDEF=/DDBG /DDEBUG /D_DEBUG
IF "%1"=="debug" SET CLOPT=/Od %CLDEF%
IF "%1"=="debug" SET LNOPT=/OPT:NOREF /OPT:NOICF
RC %CLDEF% /I..\..\..\snippets kbdcserv.rc
FOR %%i IN (bvargs bvdebug bvsque cstr memory) DO CL /nologo /c /W3 %CLOPT% /D_MBCS ..\..\..\snippets\%%i.c
CL /nologo /c /W3 %CLOPT% /D_MBCS /I..\..\..\snippets kbdcserv.c
LINK /NOLOGO %LNOPT% /RELEASE /SUBSYSTEM:CONSOLE /ENTRY:KbdcServEnter /OUT:kbdcserv.exe *.obj kbdcserv.res kernel32.lib user32.lib advapi32.lib
MODPE /NOLOGO /RELEASE /DLLFLAGS +NXCOMPAT kbdcserv.exe
:END
SET LNOPT=
SET CLOPT=
SET CLDEF=
