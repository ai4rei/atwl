@ECHO OFF
FOR %%i IN (*.obj kbdcatch.res kbdcatch.sys) DO IF EXIST %%i DEL %%i
IF "%1"=="clean" GOTO END
SET CLOPT=/O2x /GF /DNDEBUG
SET LNOPT=/OPT:REF /OPT:ICF C:\NTDDK\libfre\i386\wdm.lib
IF "%1"=="debug" SET CLOPT=/Od /DDBG /DDEBUG /D_DEBUG
IF "%1"=="debug" SET LNOPT=/OPT:NOREF /OPT:NOICF C:\NTDDK\libchk\i386\wdm.lib
RC /Fokbdcatch.res kbdcatch.rc
CL /nologo /c /W3 %CLOPT% /Gz /DWIN32 /D_WIN32 /D_X86_ /IC:\NTDDK\inc /IC:\NTDDK\inc\ddk kbdcatch.c kbdcdevs.c kbdcodev.c irpque.c
LINK /NOLOGO %LNOPT% /ENTRY:DriverEntry@8 /MACHINE:IX86 /NODEFAULTLIB /RELEASE /SUBSYSTEM:NATIVE,1.0 /OUT:kbdcatch.sys *.obj kbdcatch.res
REM /DRIVER:WDM
REM - Sets DllFlags to 0x2000 (WDM Driver)
REM - Sets Subsystem to Native.
REM - Unconditionally creates a new INIT section, that is RWEDC,
REM   which causes a warning due to a clash with alloc_text and
REM   makes the INIT section writable, which is not what we desire.
REM - Updates all non-discardable sections to be non-pagable, except
REM   PAGE*, .edata, .debug, .arch, .xdata, .sdata and .textbbs.
REM
REM So we do not use the switch, but set the subsystem ourselves and
REM then modify the sections and mark the driver as WDM with ModPE.
MODPE /NOLOGO /VERBOSE /RELEASE /DLLFLAGS +WDM /SECTIONFLAGS +INIT/D+.text/P+.rdata/P+.data/P kbdcatch.sys
:END
SET CLOPT=
SET LNOPT=
