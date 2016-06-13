
CC = c99
CFLAGS = -Wall
VPATH = ./src
O = ./obj
OBJS = $(addprefix $O/, $(addsuffix .o, main debug options io))

.SUFFIXES:

zar: $(OBJS)
	$(LINK.c) $^ -o $@

$(O)/%.o : %.c
	$(COMPILE.c) $< -o $@


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
