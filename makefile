all: copyit

copyit: copyit.c
	gcc -Wall copyit.c -o copyit

clean:
	rm -f *.o copyit
