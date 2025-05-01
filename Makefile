SRCDIR := src
BINDIR := bin

SRC := $(SRCDIR)/main.c
EXE := $(BINDIR)/qr

CC := gcc
CFLAGS := -I$(SRCDIR) -std=c23 -g3 -pedantic -Wall -Werror -Wextra -Wconversion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion

all: clean build

build: $(SRC)
	mkdir -p $(BINDIR)
	$(CC) -o $(EXE) $^ $(CFLAGS)

clean:
	rm -f $(EXE)

.PHONY: all clean build
