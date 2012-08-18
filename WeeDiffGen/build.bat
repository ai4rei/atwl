@ECHO OFF
IF "%1"=="clean" GOTO CLEAN

:SETUP
ECHO Setting up builder...
SET BUILDER=CL /nologo /W3 /O2x /FD /GF /EHsc /DWIN32 /DNDEBUG /D_WINDOWS /D_UNICODE /DUNICODE /I. %%i\%%i.cpp /link /NOLOGO /MACHINE:IX86 /DLL /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /RELEASE /OUT:%%i.dll user32.lib
IF EXIST *.dll DEL *.dll

:BUILD
ECHO Building plugins...
FOR %%i IN (WDGTemplate WDGTest) DO %BUILDER%

:TEMP
ECHO Removing temporary files...
IF EXIST *.exp DEL *.exp
IF EXIST *.lib DEL *.lib
IF EXIST *.obj DEL *.obj
IF EXIST *.idb DEL *.idb
SET BUILDER=
GOTO EXIT

:CLEAN
ECHO Cleaning up...
IF EXIST *.dll DEL *.dll
GOTO TEMP

:EXIT
