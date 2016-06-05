# Microsoft Developer Studio Project File - Name="rocred" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=rocred - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "rocred.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "rocred.mak" CFG="rocred - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "rocred - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "rocred - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "rocred - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\snippets" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x417 /i "..\snippets" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "rocred - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\snippets" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /d "_DEBUG"
# ADD RSC /l 0x417 /i "..\snippets" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"rocred.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "rocred - Win32 Release"
# Name "rocred - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bgskin.c
# End Source File
# Begin Source File

SOURCE=.\button.c
# End Source File
# Begin Source File

SOURCE=.\config.c
# End Source File
# Begin Source File

SOURCE=.\rocred.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bgskin.h
# End Source File
# Begin Source File

SOURCE=.\button.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\rocred.h
# End Source File
# Begin Source File

SOURCE=.\rsrcio.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\app-1002.ico"
# End Source File
# Begin Source File

SOURCE=".\app-1002s.ico"
# End Source File
# Begin Source File

SOURCE=.\rocred.rc
# End Source File
# End Group
# Begin Group "Snippets"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\snippets\btypes.h
# End Source File
# Begin Source File

SOURCE=..\snippets\buf.h
# End Source File
# Begin Source File

SOURCE=..\snippets\bvcstr.cpp
# End Source File
# Begin Source File

SOURCE=..\snippets\bvcstr.h
# End Source File
# Begin Source File

SOURCE=..\snippets\bvector.c
# End Source File
# Begin Source File

SOURCE=..\snippets\bvector.h
# End Source File
# Begin Source File

SOURCE=..\snippets\bvfont.cpp
# End Source File
# Begin Source File

SOURCE=..\snippets\bvfont.h
# End Source File
# Begin Source File

SOURCE=..\snippets\bvpars.cpp
# End Source File
# Begin Source File

SOURCE=..\snippets\bvpars.h
# End Source File
# Begin Source File

SOURCE=..\snippets\bvwide.cpp
# End Source File
# Begin Source File

SOURCE=..\snippets\bvwide.h
# End Source File
# Begin Source File

SOURCE=..\snippets\dlgabout.c
# End Source File
# Begin Source File

SOURCE=..\snippets\dlgabout.h
# End Source File
# Begin Source File

SOURCE=..\snippets\dlgtempl.c
# End Source File
# Begin Source File

SOURCE=..\snippets\dlgtempl.h
# End Source File
# Begin Source File

SOURCE=..\snippets\kvdb.c
# End Source File
# Begin Source File

SOURCE=..\snippets\kvdb.h
# End Source File
# Begin Source File

SOURCE=..\snippets\macaddr.c
# End Source File
# Begin Source File

SOURCE=..\snippets\macaddr.h
# End Source File
# Begin Source File

SOURCE=..\snippets\md5.c
# End Source File
# Begin Source File

SOURCE=..\snippets\md5.h
# End Source File
# Begin Source File

SOURCE=..\snippets\memory.c
# End Source File
# Begin Source File

SOURCE=..\snippets\memory.h
# End Source File
# Begin Source File

SOURCE=..\snippets\regionui.c
# End Source File
# Begin Source File

SOURCE=..\snippets\regionui.h
# End Source File
# Begin Source File

SOURCE=..\snippets\w32ui.cpp
# End Source File
# Begin Source File

SOURCE=..\snippets\w32ui.h
# End Source File
# Begin Source File

SOURCE=..\snippets\w32uxt.c
# End Source File
# Begin Source File

SOURCE=..\snippets\w32uxt.h
# End Source File
# Begin Source File

SOURCE=..\snippets\kvdb\win32ini.c
# End Source File
# Begin Source File

SOURCE=..\snippets\kvdb\win32ini.h
# End Source File
# Begin Source File

SOURCE=..\snippets\wnttools.c
# End Source File
# Begin Source File

SOURCE=..\snippets\wnttools.h
# End Source File
# Begin Source File

SOURCE=..\snippets\xf_binhex.cpp
# End Source File
# Begin Source File

SOURCE=..\snippets\xf_binhex.h
# End Source File
# Begin Source File

SOURCE=..\snippets\xf_slash.c
# End Source File
# Begin Source File

SOURCE=..\snippets\xf_slash.h
# End Source File
# End Group
# End Target
# End Project
