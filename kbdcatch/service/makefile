# Build tools
LINK = link
MODPE = modpe

# Directories
OBJ = obj
SNIPPETS = ..\..\..\snippets

# Build options
RFLAGS = -I$(SNIPPETS)

CFLAGS = \
    -nologo \
    -W3 \
    -Fo$(OBJ)\ \
    -DNDEBUG \
    -D_CRT_SECURE_NO_WARNINGS \
    -D_UNICODE \
    -D_WIN32_WINNT=0x0501 \
    -DUNICODE \
    -DNOMINMAX \
    -DBVSVCS_WITH_MAINTEPARAMS \
    -DBVSVCS_WITH_HANDLEREX \
    -DBVSVCS_VERSION=2 \
    -I$(SNIPPETS) \
    $(CFLAGS)

LFLAGS = \
    -nologo \
    -release \
    -opt:ref \
    -largeaddressaware \
    -tsaware \
    -subsystem:console,5.1 \
    kernel32.lib \
    user32.lib \
    advapi32.lib \
    $(LFLAGS)

CPPFLAGS = $(CFLAGS)

# How to suppress RC.EXE logo (except VC7)
!IF [$(RC) -nologo -? > NUL] == 0
RFLAGS = -nologo $(RFLAGS)
!ELSE
# No flags needed (VC6).
!ENDIF

KBDCSERV_EXE = kbdcserv.exe

KBDCSERV_OBJ = \
    $(OBJ)\kbdcserv.obj \
    $(OBJ)\bvdebug.obj \
    $(OBJ)\bvsque.obj \
    $(OBJ)\mem.obj \
    $(OBJ)\mem.win32.heap.obj \
    $(OBJ)\bvargs.obj \
    $(OBJ)\bvcstr.obj \
    $(OBJ)\bvsvcs.obj \
    $(OBJ)\kbdcserv.res

all : $(KBDCSERV_EXE)

clean :
    for %%i in ($(KBDCSERV_EXE)) do if exist %i del %i
    for %%i in ($(KBDCSERV_OBJ)) do if exist %i del %i

$(KBDCSERV_EXE) : $(KBDCSERV_OBJ)
    $(LINK) $(LFLAGS) -out:$@ $**
    $(MODPE) /NOLOGO /RELEASE /DLLFLAGS +NXCOMPAT $@

# Inference rules
{}.rc{$(OBJ)}.res:
    $(RC) $(RFLAGS) -Fo$@ -r $<
{}.c{$(OBJ)}.obj::
    $(CC) $(CFLAGS) -c $<
{}.cpp{$(OBJ)}.obj::
    $(CPP) $(CPPFLAGS) -c $<
!IFDEF SNIPPETS
{$(SNIPPETS)}.c{$(OBJ)}.obj::
    $(CC) $(CFLAGS) -c $<
{$(SNIPPETS)}.cpp{$(OBJ)}.obj::
    $(CPP) $(CPPFLAGS) -c $<

{$(SNIPPETS)/_mem}.c{$(OBJ)}.obj::
    $(CC) $(CFLAGS) -c $<
!ENDIF
