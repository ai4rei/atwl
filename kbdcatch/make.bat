@ECHO OFF
IF EXIST *.obj DEL *.obj
IF EXIST kbdcatch.res DEL kbdcatch.res
IF EXIST kbdcatch.sys DEL kbdcatch.sys
IF "%1"=="clean" GOTO END
RC /Fokbdcatch.res kbdcatch.rc
CL /nologo /c /W3 /IC:\NTDDK\inc /IC:\NTDDK\inc\ddk /Od /Gz /DWIN32 /D_WIN32 /D_X86_ kbdcatch.c kbdcdevs.c kbdcodev.c
LINK /NOLOGO /ENTRY:DriverEntry@8 /MACHINE:IX86 /NODEFAULTLIB /RELEASE /SUBSYSTEM:NATIVE /OUT:kbdcatch.sys *.obj kbdcatch.res C:\NTDDK\libfre\i386\ntoskrnl.lib C:\NTDDK\libfre\i386\hal.lib
:END
