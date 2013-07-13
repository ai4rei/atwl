@ECHO OFF
REM ----------------------------------------------------------------
REM RO Credentials (ROCred)
REM (c) 2012-2013 Ai4rei/AN
REM
REM ----------------------------------------------------------------

IF "%1"=="clean" GOTO CLEAN
:BUILD
RC /Forocred.res rocred.rc
CL /nologo /W3 /O1s /GF /GA /I"C:\Progra~1\Micros~1\SDK\include" /I"..\snippets" rocred.c ..\snippets\dlgabout.c ..\snippets\dlgtempl.c ..\snippets\md5.c ..\snippets\xf_binhex.c rocred.res user32.lib gdi32.lib comctl32.lib shell32.lib /link /OPT:REF /OPT:ICF /OPT:NOWIN98 /RELEASE
GOTO END
:CLEAN
FOR %%v IN (*.obj rocred.res) DO IF EXIST %%v DEL %%v
GOTO END
:END
