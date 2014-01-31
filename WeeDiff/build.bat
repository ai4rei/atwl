@ECHO OFF
REM (c) 2012 Ai4rei/AN
REM 
REM This work is licensed under a
REM Creative Commons BY-NC-SA 3.0 Unported License
REM http://creativecommons.org/licenses/by-nc-sa/3.0/

:SETUP
ECHO Setting up builder...
REM Obtain current directory
ECHO SET BUILDERDIR=^^>~~~build.bat
CD>>~~~build.bat
CALL ~~~build.bat
DEL ~~~build.bat
REM
SET OUTSTD=NUL
IF NOT "%1"=="verbose" GOTO QUIET
SHIFT
SET OUTSTD=CON
:QUIET
SET APPBUILDER=CL /nologo /c /W4 /O2x /GF /EHsc /DWIN32 /DNDEBUG /D_WINDOWS /D_UNICODE /DUNICODE /DSTRICT %%i\*.cpp /I"%BUILDERDIR%" /I"%BUILDERDIR%\Snippets" /I"%BUILDERDIR%\Common"
SET DLLBUILDER=CL /nologo /W4 /O2x /FD /GF /EHsc /DWIN32 /DNDEBUG /D_WINDOWS /D_UNICODE /DUNICODE /DSTRICT /I"%BUILDERDIR%" /I"%BUILDERDIR%\Common" %%i\*.cpp /link /NOLOGO /MACHINE:IX86 /DLL /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /RELEASE kernel32.lib user32.lib gdi32.lib comdlg32.lib
IF EXIST Release\NUL RD Release /s /q
IF "%1"=="clean" GOTO TEMP
REM Setup folders
MD Release
MD Release\plugins
MD Release\plugins\WeeDiffGen
IF NOT "%1"=="" GOTO WDG

:BUILD
ECHO Building WeeDiff...
FOR %%i IN (Snippets WeeDiff) DO %APPBUILDER% > %OUTSTD%
IF ERRORLEVEL 1 GOTO ERROR
ECHO #include "windows.h">afxres.h
RC /FoWeeDiff.res /I. WeeDiff\WeeDiff.rc > %OUTSTD%
IF ERRORLEVEL 1 GOTO ERROR
LINK /NOLOGO /MACHINE:IX86 /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /RELEASE kernel32.lib user32.lib gdi32.lib comdlg32.lib *.obj WeeDiff.res /OUT:"%BUILDERDIR%\Release\WeeDiff.exe" > %OUTSTD%
IF ERRORLEVEL 1 GOTO ERROR

ECHO Building WeeDiff plugins...
FOR %%i IN (WeeDiffPlain WeeDiffGen) DO %DLLBUILDER% /OUT:"%BUILDERDIR%\Release\plugins\%%i.dll" > %OUTSTD%

:WDG
ECHO Building WeeDiffGen plugins...
CD WeeDiffGen\Plugins
IF NOT "%1"=="" GOTO WDGINDIVL
REM Original
FOR %%i IN (WDGOnlyFirstLoginBackground WDGRemoveGravityLogo WDGSkipServiceSelect WDGEnableOfficialCustomFonts WDGEnableDNSSupport WDGDisable4LetterUserPasswordLimit WDGEnable127Hairstyles WDGAlwaysCallSelectKoreaClientInfo WDGFixCameraAnglesMedium WDGDisableMultipleWindows WDGUseRagnarokIcon WDGDisable1rag1 WDGChatAtBug WDGIncreaseZoomOut50Per WDGReadQuestTable WDGAllowMultipleWindows WDGOnlySecondLoginBackground WDGExtendedChatRoomBox WDGEnforceOfficialLoginBackground WDGSkipLicenseScreen WDGDisableSwearFilter WDGFixCameraAnglesLess WDGReadMsgStringTable WDGDisableNagleAlgorithm WDGIncreaseHeadgearViewID WDGDisable4LetterUserIDLimit WDGDisable4LetterUserCharacterLimit WDGEnableMultipleGRFExtended WDGDisableHallucinationWavyScreen WDGIgnoreMissingPaletteErrors WDGSkipResurrectionButtons WDGEnableAuraOverLvl99 WDGRemoveGravityAds WDGIncreaseZoomOutMax WDGEnableTitleBarMenu WDGAllowChatFlood WDGDisableFilenameCheck WDGCustomWindowTitle WDGDisableHShield WDGExtendedPMBox WDGLoadLUABeforeLUB WDGHKLMtoHKCU WDGExtendedChatBox WDGIncreaseZoomOut75Per WDGReadDataFolderFirst WDGUseNormalGuildBrackets WDGUseCustomAuraSprites WDGFixCameraAnglesFull WDGUsePlainTextDescriptions) DO %DLLBUILDER% /OUT:"%BUILDERDIR%\Release\plugins\WeeDiffGen\%%i.dll" > %OUTSTD%
REM Atwl
FOR %%i IN (WDGEnableAsciiInText WDGRemoveHourlyPlaytimeMinder WDGRemoveHourlyGameGrade WDGRemoveQuakeSkillEffect WDGSharedBodyPalettes WDGSkipPacketHeaderObfuscation WDGSharedHeadPalettes WDGEnable64kHairstyles WDGUseSSOLoginPacket WDGExtendedNPCDialog WDGRemoveLoginErrorCode WDGTranslateClient WDGChatColorGM WDGChatColorNormal WDGChatColorSelf WDGChatColorGuild WDGChatColorParty WDGUseCustomFont WDGScreenshotQuality WDGEnableProxySupport WDGEnableCustom3DBones) DO %DLLBUILDER% /OUT:"%BUILDERDIR%\Release\plugins\WeeDiffGen\%%i.dll"
GOTO WDGBUILT
:WDGINDIVL
FOR %%i IN (%1) DO %DLLBUILDER% /OUT:"%BUILDERDIR%\Release\plugins\WeeDiffGen\%%i.dll"
:WDGBUILT
CD "%BUILDERDIR%"

:TEMP
ECHO Removing temporary files...
IF EXIST afxres.h DEL afxres.h
IF EXIST weediff.res DEL weediff.res
IF EXIST *.obj DEL *.obj
CD Release
DEL *.exp /s > %OUTSTD%
DEL *.lib /s > %OUTSTD%
CD ..\WeeDiffGen\Plugins
DEL *.obj > %OUTSTD%
DEL *.idb > %OUTSTD%
CD "%BUILDERDIR%"
SET DLLBUILDER=
SET APPBUILDER=
SET BUILDERDIR=
SET OUT=
GOTO EXIT

:ERROR
ECHO Error occured.

:EXIT
