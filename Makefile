# modify this makefile for your implementation
#   as described in the handout

sq.o: sq.h sq.c
	gcc -c sq.c

fdriver: driver.c sq.o 
	gcc driver.c sq.o -lc -o fdriver

clean:
	rm -f *.o fdriver
