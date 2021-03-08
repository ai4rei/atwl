all : WDG*

WDG* : UpdateAlways
    if exist $@\Makefile <<nmaketmp.bat
    @echo off
    cd $@
    $(MAKE) -nologo -$(MAKEFLAGS) "BIN_DIR=$(BIN_DIR)" "OBJ_DIR=$(OBJ_DIR)" "CPPFLAGS=$(CPPFLAGS) -I$(BASE_DIR)\Common" "LDFLAGS=$(LDFLAGS)" "PLUGINNAME=$@"
    cd ..
<<

UpdateAlways:
