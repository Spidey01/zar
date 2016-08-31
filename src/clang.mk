CC = clang
CFLAGS = -Wall
LDFLAGS = -lz

ifeq ($(debug),true)
CFLAGS = -O0 -g
endif

include src/unix.mk
