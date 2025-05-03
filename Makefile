SRCDIR := src
OBJDIR := obj
BINDIR := bin

SRC := $(SRCDIR)/bv.c $(SRCDIR)/gf256.c $(SRCDIR)/qr.c $(SRCDIR)/main.c
OBJ := $(OBJDIR)/bv.o $(OBJDIR)/gf256.o $(OBJDIR)/qr.o $(OBJDIR)/main.o
EXE := $(BINDIR)/qr

CC := gcc
CFLAGS :=  -DBUILD_DEBUG -I$(SRCDIR) -std=c23 -g3 -pedantic -Wall -Werror -Wextra -Wconversion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion

all: clean build

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

build: $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) -o $(EXE) $^ $(CFLAGS)

clean:
	rm -f $(OBJDIR)/*.o $(EXE)

.PHONY: all clean build
