
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
	@echo Test...creating ZAR archive
	./zar -c -f wtf.zar tmp/eggs tmp/ham tmp/quux tmp/spam
	@echo Test...listing ZAR archive
	./zar -t -f wtf.zar

