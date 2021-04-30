CFLAGS=-lraylib -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces

gol: main.c
	${CC} ${CFLAGS} main.c -o gol

run: gol
	./gol

.PHONY: clean

clean:
	${RM} *.o gol
