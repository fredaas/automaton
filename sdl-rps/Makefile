CFLAGS+=-std=c99 -O2 -fopenmp -g

all:
	gcc -o main main.c $(shell sdl2-config --cflags --libs) $(CFLAGS)

run :
	@./main

format :
	clang-format-6.0 -style=file -i *.c *.h

clean:
	-rm -f *.o main
