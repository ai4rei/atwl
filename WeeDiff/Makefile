LINK = link

# Normalize MAKEDIR so that appends do not result in \\ separator.
BASE_DIR = $(MAKEDIR)\.
BASE_DIR = $(BASE_DIR:^\^\=^\)

CPPFLAGS = -nologo -W3 -O2x -GF -EHsc -DWIN32 -DNDEBUG -D_WINDOWS -D_UNICODE -DUNICODE -DSTRICT -I$(BASE_DIR)
LFLAGS = -nologo -opt:ref -opt:icf -release kernel32.lib user32.lib gdi32.lib comdlg32.lib comctl32.lib
LDFLAGS = $(LFLAGS) -dll

# Detect whether -nologo is supported and necessary (does not work with MSVC7)
!IF [$(RC) -nologo -? > NUL]==0
RFLAGS = -nologo $(RFLAGS)
!ENDIF

BIN_DIR = Bin
OBJ_DIR = Obj
SNIPPETS_DIR = Snippets
WEEDIFF_DIR = WeeDiff
WEEDIFFGEN_DIR = WeeDiffGen
WEEDIFFGENPLUGIN_DIR = $(WEEDIFFGEN_DIR)\Plugins
WEEDIFFPLAIN_DIR = WeeDiffPlain

SNIPPETS_H = \
    $(SNIPPETS_DIR)\*.h

SNIPPETS_CPP = \
    $(SNIPPETS_DIR)\*.cpp

SNIPPETS_OBJ = \
    $(OBJ_DIR)\Control.obj \
    $(OBJ_DIR)\Toolbar.obj \
    $(OBJ_DIR)\WeeMessageBox.obj \
    $(OBJ_DIR)\Window.obj

WEEDIFF_H = \
    $(WEEDIFF_DIR)\*.h

WEEDIFF_CPP = \
    $(WEEDIFF_DIR)\*.cpp

# FIXME: There is a file name conflict.
WEEDIFF_OBJ = \
    $(OBJ_DIR)\WeeDiff.obj \
    $(OBJ_DIR)\WeeDiff.res \
    $(OBJ_DIR)\WeePluginManager.obj

WEEDIFF_BIN = \
    $(BIN_DIR)\WeeDiff.exe

WEEDIFF_RESOURCES = \
    $(SNIPPETS_DIR)\WeeMessageBox.bmp \
    $(SNIPPETS_DIR)\WeeTools.bmp \
    $(WEEDIFF_DIR)\checkboxes.bmp \
    $(WEEDIFF_DIR)\icons.ico \
    $(WEEDIFF_DIR)\imagelist.bmp \
    $(WEEDIFF_DIR)\WeeDiff.manifest

WEEDIFF_RC = \
    $(WEEDIFF_DIR)\WeeDiff.rc

WEEDIFFGEN_H = \
    $(WEEDIFFGEN_DIR)\*.h

WEEDIFFGEN_CPP = \
    $(WEEDIFFGEN_DIR)\*.cpp

WEEDIFFGEN_OBJ = \
    $(OBJ_DIR)\$(WEEDIFFGEN_DIR)\RagExe.obj \
    $(OBJ_DIR)\$(WEEDIFFGEN_DIR)\WeeDiffCallback.obj \
    $(OBJ_DIR)\$(WEEDIFFGEN_DIR)\WeeDiffGen.obj \
    $(OBJ_DIR)\$(WEEDIFFGEN_DIR)\WeeInputBox.obj \
    $(OBJ_DIR)\$(WEEDIFFGEN_DIR)\WeePluginManager.obj \
    $(OBJ_DIR)\$(WEEDIFFGEN_DIR)\WeeUtility.obj

WEEDIFFGEN_BIN = \
    $(BIN_DIR)\Plugins\WeeDiffGen.dll

WEEDIFFPLAIN_H = \
    $(WEEDIFFPLAIN_DIR)\*.h

WEEDIFFPLAIN_CPP = \
    $(WEEDIFFPLAIN_DIR)\*.cpp

WEEDIFFPLAIN_OBJ = \
    $(OBJ_DIR)\WeeDiffPlain.obj

WEEDIFFPLAIN_BIN = \
    $(BIN_DIR)\Plugins\WeeDiffPlain.dll

all : WeeDiff WeeDiffGen WeeDiffPlain

clean : CleanWeeDiffPlain CleanWeeDiffGen CleanWeeDiff CleanSnippets CleanAfxRes.h CleanObj CleanBin

$(SNIPPETS_OBJ) $(WEEDIFF_OBJ) $(WEEDIFFGEN_OBJ) $(WEEDIFFPLAIN_OBJ) : $(BIN_DIR) $(OBJ_DIR) Makefile AfxRes.h

$(SNIPPETS_CPP) : $(SNIPPETS_H)

CleanSnippets :
    for %i in ($(SNIPPETS_OBJ)) do if exist %i del %i

WeeDiff : $(WEEDIFF_BIN)

$(WEEDIFF_BIN) : $(WEEDIFF_OBJ) $(SNIPPETS_OBJ)
    $(LINK) $(LFLAGS) -out:$@ $**

$(WEEDIFF_CPP) : $(WEEDIFF_H)

$(WEEDIFF_RC) : $(WEEDIFF_RESOURCES)

CleanWeeDiff :
    for %i in ($(WEEDIFF_OBJ)) do if exist %i del %i
    for %i in ($(WEEDIFF_BIN)) do if exist %i del %i

WeeDiffGen : $(WEEDIFFGEN_BIN) WeeDiffGenPlugins

$(WEEDIFFGEN_BIN) : $(WEEDIFFGEN_OBJ)
    $(LINK) $(LDFLAGS) -out:$@ $**
    for %i in ($(WEEDIFFGEN_BIN:.dll=.exp)) do if exist %i del %i
    for %i in ($(WEEDIFFGEN_BIN:.dll=.lib)) do if exist %i del %i

$(WEEDIFFGEN_CPP) : $(WEEDIFFGEN_H)

CleanWeeDiffGen :
    for %i in ($(WEEDIFFGEN_OBJ)) do if exist %i del %i
    for %i in ($(WEEDIFFGEN_BIN)) do if exist %i del %i
    for %i in ($(WEEDIFFGEN_BIN:.dll=.exp)) do if exist %i del %i
    for %i in ($(WEEDIFFGEN_BIN:.dll=.lib)) do if exist %i del %i
    if exist $(BIN_DIR)\Plugins\WeeDiffGen\WDG*.dll del $(BIN_DIR)\Plugins\WeeDiffGen\WDG*.dll
    if exist $(BIN_DIR)\Plugins\WeeDiffGen\WDG*.exp del $(BIN_DIR)\Plugins\WeeDiffGen\WDG*.exp
    if exist $(BIN_DIR)\Plugins\WeeDiffGen\WDG*.lib del $(BIN_DIR)\Plugins\WeeDiffGen\WDG*.lib
    if exist $(OBJ_DIR)\WDG*.obj del $(OBJ_DIR)\WDG*.obj

WeeDiffGenPlugins :
    cd $(WEEDIFFGENPLUGIN_DIR)
    $(MAKE) -nologo -$(MAKEFLAGS) "BASE_DIR=$(BASE_DIR)" "BIN_DIR=$(BASE_DIR)\$(BIN_DIR)\Plugins\WeeDiffGen" "OBJ_DIR=$(BASE_DIR)\$(OBJ_DIR)" "CPPFLAGS=$(CPPFLAGS)" "LDFLAGS=$(LDFLAGS)"
    cd $(BASE_DIR)
    if exist $(BIN_DIR)\Plugins\WeeDiffGen\WDG*.exp del $(BIN_DIR)\Plugins\WeeDiffGen\WDG*.exp
    if exist $(BIN_DIR)\Plugins\WeeDiffGen\WDG*.lib del $(BIN_DIR)\Plugins\WeeDiffGen\WDG*.lib

WeeDiffPlain : $(WEEDIFFPLAIN_BIN)

$(WEEDIFFPLAIN_BIN) : $(WEEDIFFPLAIN_OBJ)
    $(LINK) $(LDFLAGS) -out:$@ $**
    for %i in ($(WEEDIFFPLAIN_BIN:.dll=.exp)) do if exist %i del %i
    for %i in ($(WEEDIFFPLAIN_BIN:.dll=.lib)) do if exist %i del %i

$(WEEDIFFPLAIN_CPP) : $(WEEDIFFPLAIN_H)

CleanWeeDiffPlain :
    for %i in ($(WEEDIFFPLAIN_OBJ)) do if exist %i del %i
    for %i in ($(WEEDIFFPLAIN_BIN)) do if exist %i del %i
    for %i in ($(WEEDIFFPLAIN_BIN:.dll=.exp)) do if exist %i del %i
    for %i in ($(WEEDIFFPLAIN_BIN:.dll=.lib)) do if exist %i del %i

$(BIN_DIR) :
    if not exist $(BIN_DIR)\nul                     mkdir $(BIN_DIR)
    if not exist $(BIN_DIR)\Plugins\nul             mkdir $(BIN_DIR)\Plugins
    if not exist $(BIN_DIR)\Plugins\WeeDiffGen\nul  mkdir $(BIN_DIR)\Plugins\WeeDiffGen

CleanBin :
    if exist $(BIN_DIR)\Plugins\WeeDiffGen\NUL      rmdir $(BIN_DIR)\Plugins\WeeDiffGen
    if exist $(BIN_DIR)\Plugins\NUL                 rmdir $(BIN_DIR)\Plugins
    if exist $(BIN_DIR)\NUL                         rmdir $(BIN_DIR)

$(OBJ_DIR) :
    if not exist $(OBJ_DIR)\NUL                     mkdir $(OBJ_DIR)
    if not exist $(OBJ_DIR)\$(WEEDIFFGEN_DIR)\NUL   mkdir $(OBJ_DIR)\$(WEEDIFFGEN_DIR)

CleanObj :
    if exist $(OBJ_DIR)\$(WEEDIFFGEN_DIR)\NUL       rmdir $(OBJ_DIR)\$(WEEDIFFGEN_DIR)
    if exist $(OBJ_DIR)\NUL                         rmdir $(OBJ_DIR)

AfxRes.h :
    if not exist afxres.h rem <<afxres.h
    #include <windows.h>
<<KEEP

CleanAfxRes.h :
    if exist afxres.h del afxres.h

{$(SNIPPETS_DIR)}.cpp{$(OBJ_DIR)}.obj::
    $(CPP) $(CPPFLAGS) -c -Fo$(OBJ_DIR)\ $<

{$(WEEDIFF_DIR)}.cpp{$(OBJ_DIR)}.obj::
    $(CPP) $(CPPFLAGS) -c -Fo$(OBJ_DIR)\ $<

{$(WEEDIFF_DIR)}.rc{$(OBJ_DIR)}.res:
    $(RC) $(RFLAGS) -Fo$@ $<

{$(WEEDIFFGEN_DIR)}.cpp{$(OBJ_DIR)\$(WEEDIFFGEN_DIR)}.obj::
    $(CPP) $(CPPFLAGS) -c -Fo$(OBJ_DIR)\$(WEEDIFFGEN_DIR)\ $<

{$(WEEDIFFPLAIN_DIR)}.cpp{$(OBJ_DIR)}.obj::
    $(CPP) $(CPPFLAGS) -c -Fo$(OBJ_DIR)\ $<
