INCDIR := inc
SRCDIR := src
OBJDIR := obj

INC := $(INCDIR)/bv.h $(INCDIR)/gf256.h $(INCDIR)/qr.h $(INCDIR)/utils.h
SRC := $(SRCDIR)/bv.c $(SRCDIR)/gf256.c $(SRCDIR)/qr.c $(SRCDIR)/main.c
OBJ := $(OBJDIR)/bv.o $(OBJDIR)/gf256.o $(OBJDIR)/qr.o $(OBJDIR)/main.o

EXE := qr

CC := gcc
CFLAGS := -I$(INCDIR) -std=c23 -g3 -pedantic -Wall -Werror -Wextra -Wconversion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion

all: clean build

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INC)
	$(CC) -c -o $@ $< $(CFLAGS)

build: $(OBJ)
	mkdir -p $(OBJDIR)
	$(CC) -o $(EXE) $^ $(CFLAGS)

clean:
	rm -f $(OBJDIR)/*.o $(EXE)

.PHONY: all clean build
