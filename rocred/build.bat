@ECHO OFF
REM ----------------------------------------------------------------
REM RO Credentials (ROCred)
REM (c) 2012-2013 Ai4rei/AN
REM
REM ----------------------------------------------------------------

RC /Forocred.res rocred.rc
CL /W3 /O1s /GF /GA /I"C:\Progra~1\Micros~1\SDK\include" /I"..\snippets" rocred.c ..\snippets\dlgabout.c ..\snippets\dlgtempl.c ..\snippets\md5.c ..\snippets\xf_binhex.c rocred.res user32.lib gdi32.lib comctl32.lib /link /OPT:REF /OPT:ICF /OPT:NOWIN98 /RELEASE
