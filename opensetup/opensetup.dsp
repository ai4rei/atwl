# Microsoft Developer Studio Project File - Name="opensetup" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=opensetup - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "opensetup.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "opensetup.mak" CFG="opensetup - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "opensetup - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "opensetup - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "opensetup - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /vd0 /GX /Ox /Ot /Og /Oi /Gy /I "snippets" /I "lib/lua" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /D "HAVE_LUA514" /D "DX7E_DYNAMIC" /FD /GF /c
# SUBTRACT CPP /Os /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /Oicf /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG" /d "HAVE_LUA514"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 lua.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib htmlhelp.lib /nologo /version:1.0 /subsystem:windows /machine:I386 /out:"opensetup.exe" /libpath:"lib/lua" /release /opt:ref /opt:icf /opt:nowin98 /tsaware
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "opensetup - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /Gi /GX /ZI /Od /I "snippets" /I "lib/lua" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /D "HAVE_LUA514" /D "DX7E_DYNAMIC" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /Oicf /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x405 /d "_DEBUG"
# ADD RSC /l 0x417 /d "_DEBUG" /d "HAVE_LUA514"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 luad.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib htmlhelp.lib /nologo /subsystem:windows /debug /machine:I386 /out:"opensetup.exe" /pdbtype:sept /libpath:"lib/lua"

!ENDIF 

# Begin Target

# Name "opensetup - Win32 Release"
# Name "opensetup - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\dx7enum.cpp
# End Source File
# Begin Source File

SOURCE=.\luaio.cpp
# End Source File
# Begin Source File

SOURCE=.\opensetup.cpp
# End Source File
# Begin Source File

SOURCE=.\roext.cpp
# End Source File
# Begin Source File

SOURCE=.\settings.cpp
# End Source File
# Begin Source File

SOURCE=.\settings_lua.cpp
# End Source File
# Begin Source File

SOURCE=.\settings_reg.cpp
# End Source File
# Begin Source File

SOURCE=.\tab.cpp
# End Source File
# Begin Source File

SOURCE=.\ui.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\dx7enum.h
# End Source File
# Begin Source File

SOURCE=.\luaio.h
# End Source File
# Begin Source File

SOURCE=.\opensetup.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\roext.h
# End Source File
# Begin Source File

SOURCE=.\settings.h
# End Source File
# Begin Source File

SOURCE=.\settings_lua.h
# End Source File
# Begin Source File

SOURCE=.\settings_reg.h
# End Source File
# Begin Source File

SOURCE=.\tab.h
# End Source File
# Begin Source File

SOURCE=.\ui.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\appicon_l.ico
# End Source File
# Begin Source File

SOURCE=.\res\appicon_s.ico
# End Source File
# Begin Source File

SOURCE=.\res\engine_lua.ico
# End Source File
# Begin Source File

SOURCE=.\res\engine_reg.ico
# End Source File
# Begin Source File

SOURCE=".\res\imagelist-mask.bmp"
# End Source File
# Begin Source File

SOURCE=.\res\imagelist.bmp
# End Source File
# Begin Source File

SOURCE=.\res\imagelist32.bmp
# End Source File
# Begin Source File

SOURCE=.\res\opensetup.rc
# End Source File
# End Group
# Begin Group "Snippets"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\snippets\regutil.cpp
# End Source File
# Begin Source File

SOURCE=.\snippets\regutil.h
# End Source File
# End Group
# End Target
# End Project
