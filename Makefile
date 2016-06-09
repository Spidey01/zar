
help:
	@echo wtf

all: zar test


zar:
	$(MAKE) /NOLOGO -f src/$(toolchain).mk

test:
	.\zar.exe -c -f wtf.zar tmp\eggs tmp\ham tmp\quux tmp\spam
	.\zar.exe -t -f wtf.zar

old-test:
	-.\zar.exe -h
	-.\zar.exe --help
	 .\zar.exe -f foo.zar -c obj src
	 .\zar.exe
	 .\zar.exe -c obj src
