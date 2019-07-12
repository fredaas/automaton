CFLAGS=-std=c99 -O0 -Wall -lm
CC=gcc

all: clean main

main : main.c
	$(CC) -o $@ $^ $(shell sdl2-config --cflags --libs) $(CFLAGS)

run :
	@./main

clean:
	-rm -f *.o main
