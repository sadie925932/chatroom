CC=gcc
CFLAGS=-O3
LIBS=-pthread
SRC=src

all: server.out client.out
server.out: server.o
	gcc -O3 -pthread -o server.out server.o
client.out: client.o string.o
	gcc -O3 -pthread -o client.out client.o string.o
server.o: server.c
	gcc -O3 -c server.c
string.o: string.c
	gcc -O3 -c string.c
client.o: client.c
	gcc -O3 -c client.c

.PHONY: clean
clean:
	rm -f *.o *.out
