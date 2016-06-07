
LINK = link /NOLOGO
CC = cl /nologo
CFLAGS = /W4 /MT /GR /EHsc
O = .\obj
S = .\src

all: zar.exe

zar.exe: $O/main.obj $O/debug.obj $O/options.obj
	$(CC) $(CFLAGS) /Fe$@ $**

{$S}.c{$O}.obj::
	$(CC) $(CFLAGS) /Fo$(O)\ /c $<

