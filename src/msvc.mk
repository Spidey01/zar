
LINK = link /NOLOGO
CC = cl /nologo
CFLAGS = /W4 /MD /GR /EHsc /D_CRT_SECURE_NO_WARNINGS /I.\\zlib\\ 
LIBS = zlib.lib
O = .\obj
S = .\src

zar: zar.exe

zar.exe: $O/main.obj $O/debug.obj $O/options.obj $O/io.obj 
	$(CC) $(CFLAGS) /Fe$@ $** $O\zlib.lib

{$S}.c{$O}.obj::
	$(CC) $(CFLAGS) /Fo$(O)\ /c $<

clean:
	-DEL /Q $O\*.obj
	-DEL .\zar.exe

test:
	@echo Test...creating ZAR archive
	.\zar.exe -c -f wtf.zar tmp\eggs tmp\ham tmp\quux tmp\spam
	@echo Test...listing ZAR archive
	.\zar.exe -t -f wtf.zar

