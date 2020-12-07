@ECHO OFF
REM ----------------------------------------------------------------
REM RO Credentials (ROCred)
REM (c) 2012+ Ai4rei/AN
REM
REM ----------------------------------------------------------------

:CLEAN
FOR %%v IN (obj\*.obj obj\rocred.res) DO IF EXIST %%v DEL %%v
IF NOT EXIST obj\NUL MD obj
IF "%1"=="clean" GOTO END
:BUILD
RC /Foobj\rocred.res /I..\snippets rocred.rc
CL @rocred.build
MODPE /NOLOGO /RELEASE /DLLFLAGS +DYNAMICBASE+NXCOMPAT+TSAWARE rocred.exe
:END
