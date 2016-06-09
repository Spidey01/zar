
CC = c99
CFLAGS = -Wall
VPATH = ./src
O = ./obj
OBJS = $(addprefix $O/, $(addsuffix .o, main debug options io))

.SUFFIXES:

all: zar

zar: $(OBJS)
	$(LINK.c) $^ -o $@

$(O)/%.o : %.c
	$(COMPILE.c) $< -o $@


clean:
	rm -f $O/*.o
	rm -f zar
