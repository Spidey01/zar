
CC = gcc
CFLAGS = -Wall
#
# Should this be in LDFLAGS or LDLIBS? I think the latter
# but LINK.c doesn't use LDLIBS!
#
LDFLAGS = -lz

include src/unix.mk
