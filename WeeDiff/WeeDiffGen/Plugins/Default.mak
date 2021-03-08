LINK = link

PLUGIN_H = $(PLUGINNAME).h
PLUGIN_CPP = $(PLUGINNAME).cpp
PLUGIN_OBJ = $(OBJ_DIR)\$(PLUGINNAME).obj
PLUGIN_BIN = $(BIN_DIR)\$(PLUGINNAME).dll

all : $(PLUGIN_BIN)

clean :
    for %i in ($(PLUGIN_OBJ)) do if exist %i del %i
    for %i in ($(PLUGIN_BIN)) do if exist %i del %i
    for %i in ($(PLUGIN_BIN:.dll=.exp)) do if exist %i del %i
    for %i in ($(PLUGIN_BIN:.dll=.lib)) do if exist %i del %i

$(PLUGIN_BIN) : $(PLUGIN_OBJ)
    $(LINK) $(LDFLAGS) -out:$@ $**

$(PLUGIN_CPP) : $(PLUGIN_H)

{.}.cpp{$(OBJ_DIR)}.obj::
    $(CPP) $(CPPFLAGS) -c -Fo$(OBJ_DIR)\ $<
