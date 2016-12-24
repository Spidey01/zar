#
# $(toolchain).mk's targeting UNIX systems should include this file.
#

VPATH = ./src
O = ./obj
LIBZ = $(O)/lib/libz.a
OBJS = $(addprefix $O/, $(addsuffix .o, system main debug options io)) $(LIBZ)

.SUFFIXES:

zar: $(OBJS)
	$(LINK.c) $^ -o $@

$(O)/%.o : %.c $(O)
	$(COMPILE.c) $< -o $@

$(O):
	mkdir $@

FIXZLIB = cd zlib && git reset --hard HEAD && git clean -f
$(LIBZ): $(O) zlib/libz.a

$(LIBZ):
	$(FIXZLIB)
	cd zlib && ./configure --static --prefix=$(CURDIR)/$O/
	cd zlib && make install
	$(FIXZLIB)

clean:
	rm -f $O/*.o
	rm -f zar

test:
	rm -f wtf.zar
	@echo Test...creating ZAR archive
	./zar -c -f wtf.zar tmp/eggs tmp/ham tmp/quux tmp/spam
	@echo Test...listing ZAR archive
	./zar -t -f wtf.zar
	@echo Test...extracting ZAR archive
	rm -rf wtf.test
	-mkdir wtf.test
	-mkdir wtf.test\tmp
	./zar -x -f wtf.zar -C wtf.test
