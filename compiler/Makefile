CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -D_POSIX_C_SOURCE=200809L

BISON = bison
FLEX = flex

OBJS = assembler.tab.o lex.yy.o asm.o main.o

all: assembler

assembler: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lfl

assembler.tab.c assembler.tab.h: assembler.y
	$(BISON) -d -o assembler.tab.c assembler.y

lex.yy.c: assembler.l assembler.tab.h
	$(FLEX) -o lex.yy.c assembler.l

assembler.tab.o: assembler.tab.c asm.h
	$(CC) $(CFLAGS) -c assembler.tab.c

lex.yy.o: lex.yy.c assembler.tab.h
	$(CC) $(CFLAGS) -c lex.yy.c

asm.o: asm.c asm.h
	$(CC) $(CFLAGS) -c asm.c

main.o: main.c asm.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f assembler assembler.tab.c assembler.tab.h lex.yy.c *.o

.PHONY: all clean
