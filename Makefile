
help:
	@echo targets: help zlib zar test
	@echo help: this help.
	@echo zlib: build zlib dependency (Windows)
	@echo zar: build zar.
	@echo test: run tests on zar.

all: zlib zar test

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

zlib: FORCE
	$(MAKE) -f zlib.$(toolchain).mk O=./obj $@

# Not all makes support .PHONY, and nmake is one of them?
FORCE:

