
help:
	@echo targets: help zlib zar test
	@echo help: this help.
	@echo zlib: build zlib dependency (Windows)
	@echo zar: build zar.
	@echo test: run tests on zar.

all: zlib zar test

zar test clean: FORCE
	$(MAKE) -f src/$(toolchain).mk $@

zlib: FORCE
	$(MAKE) -f zlib.$(toolchain).mk O=./obj $@

# Not all makes support .PHONY, and nmake is one of them?
FORCE:

