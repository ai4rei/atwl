@ECHO OFF

:SETUP
ECHO Setting up builder...
REM Obtain current directory
ECHO SET BUILDERDIR=^^>~~~build.bat
CD>>~~~build.bat
CALL ~~~build.bat
DEL ~~~build.bat
REM
SET APPBUILDER=CL /nologo /c /W3 /O2x /GF /EHsc /DWIN32 /DNDEBUG /D_WINDOWS /D_UNICODE /DUNICODE %%i\*.cpp /I"%BUILDERDIR%" /I"%BUILDERDIR%\Snippets" /I"%BUILDERDIR%\WeeDiff\Common"
SET DLLBUILDER=CL /nologo /W3 /O2x /FD /GF /EHsc /DWIN32 /DNDEBUG /D_WINDOWS /D_UNICODE /DUNICODE /I"%BUILDERDIR%\WeeDiff\Common" %%i\*.cpp /link /NOLOGO /MACHINE:IX86 /DLL /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /RELEASE kernel32.lib user32.lib gdi32.lib comdlg32.lib
IF EXIST Release\NUL RD Release /s /q
MD Release
MD Release\plugins
MD Release\plugins\WeeDiffGen
REM GOTO WDG

:BUILD
ECHO Building WeeDiff...
FOR %%i IN (Snippets WeeDiff\WeeDiff) DO %APPBUILDER% > NUL
IF ERRORLEVEL 1 GOTO ERROR
ECHO #include "windows.h">afxres.h
RC /FoWeeDiff.res /I. WeeDiff\WeeDiff\WeeDiff.rc > NUL
IF ERRORLEVEL 1 GOTO ERROR
LINK /NOLOGO /MACHINE:IX86 /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /RELEASE kernel32.lib user32.lib gdi32.lib comdlg32.lib *.obj WeeDiff.res /OUT:"%BUILDERDIR%\Release\WeeDiff.exe" > NUL
IF ERRORLEVEL 1 GOTO ERROR

ECHO Building WeeDiff plugins...
CD WeeDiff
FOR %%i IN (WeeDiffPlain WeeDiffGen) DO %DLLBUILDER% /OUT:"%BUILDERDIR%\Release\plugins\%%i.dll" > NUL
CD "%BUILDERDIR%"

:WDG
ECHO Building WeeDiffGen plugins...
CD WeeDiff\WeeDiffGen\WeeDiffGen
REM Original
FOR %%i IN (WDGOnlyFirstLoginBackground WDGRemoveGravityLogo WDGSkipServiceSelect WDGEnableOfficialCustomFonts WDGEnableDNSSupport WDGDisable4LetterUserPasswordLimit WDGEnable127Hairstyles WDGAlwaysCallSelectKoreaClientInfo WDGFixCameraAnglesMedium WDGDisableMultipleWindows WDGUseRagnarokIcon WDGDisable1rag1 WDGChatAtBug WDGIncreaseZoomOut50Per WDGRestoreLoginWindow WDGReadQuestTable WDGAllowMultipleWindows WDGOnlySecondLoginBackground WDGExtendedChatRoomBox WDGEnforceOfficialLoginBackground WDGSkipLicenseScreen WDGIgnoreMissingFileErrors WDGDisableSwearFilter WDGFixCameraAnglesLess WDGReadMsgStringTable WDGDisableNagleAlgorithm WDGUseArialOnAllLangtypes WDGIncreaseHeadgearViewID WDGDisable4LetterUserIDLimit WDGDisable4LetterUserCharacterLimit WDGEnableMultipleGRFExtended WDGDisableHallucinationWavyScreen WDGIgnoreMissingPaletteErrors WDGSkipResurrectionButtons WDGEnableAuraOverLvl99 WDGRemoveGravityAds WDGIncreaseZoomOutMax WDGTranslateClientIntoEnglish WDGEnableMultipleGRF WDGEnableTitleBarMenu WDGAllowChatFlood WDGDisableFilenameCheck WDGCustomWindowTitle WDGDisableHShield WDGExtendedPMBox WDGLoadLUABeforeLUB WDGHKLMtoHKCU WDGExtendedChatBox WDGIncreaseZoomOut75Per WDGReadDataFolderFirst WDGUseNormalGuildBrackets WDGUseCustomAuraSprites WDGFixCameraAnglesFull WDGUsePlainTextDescriptions) DO %DLLBUILDER% /OUT:"%BUILDERDIR%\Release\plugins\WeeDiffGen\%%i.dll" > NUL
REM Atwl
FOR %%i IN (WDGEnableAsciiInText WDGRemoveHourlyPlaytimeMinder WDGSharedBodyPalettes) DO %DLLBUILDER% /OUT:"%BUILDERDIR%\Release\plugins\WeeDiffGen\%%i.dll"
CD "%BUILDERDIR%"

:TEMP
ECHO Removing temporary files...
IF EXIST afxres.h DEL afxres.h
DEL *.exp /s > NUL
DEL *.lib /s > NUL
DEL *.obj /s > NUL
DEL *.idb /s > NUL
SET DLLBUILDER=
SET APPBUILDER=
SET BUILDERDIR=
GOTO EXIT

:ERROR
ECHO Error occured.

:EXIT
