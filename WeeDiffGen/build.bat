@ECHO OFF
SET BUILDER=CL /nologo /W3 /O2x /GD /FD /GF /GX /DWIN32 /DNDEBUG /D_WINDOWS /D_UNICODE /DUNICODE /I. %%i\%%i.cpp /link /NOLOGO /MACHINE:IX86 /DLL /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /RELEASE /OUT:%%i.dll user32.lib
IF EXIST *.dll DEL *.dll
ECHO Building plugins...
FOR %%i IN (WDGTemplate WDGTest) DO %BUILDER%
IF EXIST *.exp DEL *.exp
IF EXIST *.lib DEL *.lib
IF EXIST *.obj DEL *.obj
IF EXIST *.idb DEL *.idb
SET BUILDER=
