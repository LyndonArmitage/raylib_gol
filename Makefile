CFLAGS=-lraylib -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces

gol: main.c gifenc.c gifenc.h
	${CC} ${CFLAGS} main.c gifenc.h gifenc.c -o gol

run: gol
	./gol

.PHONY: clean

clean:
	${RM} *.o gol
