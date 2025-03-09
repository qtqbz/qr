CC := gcc
CFLAGS := -I. -std=c23 -g3 -pedantic -Wall -Werror -Wextra -Wconversion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion

all: clean build

build: qr.c
	$(CC) -o qr qr.c $(CFLAGS)

build_debug: qr.c
	$(CC) -o qr qr.c $(CFLAGS) -DBUILD_DEBUG=1

clean:
	rm -f *.o qr

.PHONY: all clean build build_debug
.SILENT: all clean build build_debug
