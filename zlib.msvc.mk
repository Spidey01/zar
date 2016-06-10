
# Build a static zlib for MS Visual C and dump it into obj.
zlib: $O/zlib.lib

$O/zlib.lib:
	CD zlib && $(MAKE) /$(MAKEFLAGS) /F .\win32\Makefile.msc zlib.lib
	COPY /B /Y zlib\zlib.lib obj\\
	CD zlib && $(MAKE) /$(MAKEFLAGS) /F .\win32\Makefile.msc /I clean

