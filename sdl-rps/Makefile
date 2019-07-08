CFLAGS+=-std=c99 -O2 -Wall -fopenmp

all:
	gcc -o main main.c $(shell sdl2-config --cflags --libs) $(CFLAGS)

run :
	@./main

clean:
	-rm -f *.o main
