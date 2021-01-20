@ECHO OFF
RC -Fohotfix32.res hotfix32.rc
CL -nologo -W3 -LD -O2x -GF -D_CRT_SECURE_NO_WARNINGS hotfix32.c hotfix32.res -link -def:hotfix32.def
