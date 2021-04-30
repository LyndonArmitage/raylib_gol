CFLAGS=-lraylib -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces

all:
	${CC} ${CFLAGS} main.c -o gol

run: all
	./gol

.PHONY: clean

clean:
	${RM} *.o gol
