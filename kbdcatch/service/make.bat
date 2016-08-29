@ECHO OFF
FOR %%i IN (*.obj *.res) DO IF EXIST %%i DEL %%i
IF "%1"=="clean" GOTO END
SET CLDEF=/DNDEBUG /DBVSVCS_WITH_MAINTEPARAMS /DBVSVCS_WITH_HANDLEREX /DBVSVCS_VERSION=2
SET CLOPT=/O2x /GF %CLDEF%
SET LNOPT=/OPT:REF /OPT:ICF /OPT:NOWIN98
IF "%1"=="debug" SET CLDEF=/DDBG /DDEBUG /D_DEBUG /DBVSVCS_WITH_MAINTEPARAMS /DBVSVCS_WITH_HANDLEREX /DBVSVCS_VERSION=2
IF "%1"=="debug" SET CLOPT=/Od %CLDEF%
IF "%1"=="debug" SET LNOPT=/OPT:NOREF /OPT:NOICF
RC %CLDEF% /I..\..\..\snippets /Fokbdcserv.res kbdcserv.rc
FOR %%i IN (bvdebug bvsque memory) DO CL /nologo /c /W3 %CLOPT% /D_MBCS ..\..\..\snippets\%%i.c
FOR %%i IN (bvargs bvcstr bvsvcs) DO CL /nologo /c /W3 %CLOPT% /D_MBCS ..\..\..\snippets\%%i.cpp
CL /nologo /c /W3 %CLOPT% /D_MBCS /D_WIN32_WINNT=0x0501 /I..\..\..\snippets kbdcserv.c
LINK /NOLOGO %LNOPT% /RELEASE /SUBSYSTEM:CONSOLE,5.1 /OUT:kbdcserv.exe *.obj kbdcserv.res kernel32.lib user32.lib advapi32.lib
MODPE /NOLOGO /RELEASE /DLLFLAGS +NXCOMPAT kbdcserv.exe
:END
SET LNOPT=
SET CLOPT=
SET CLDEF=
