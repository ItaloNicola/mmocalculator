build:
	gcc -Wall main.c networking.c unicode.c `pkg-config --cflags --libs gtk+-3.0` -o calc -lm

server:
	gcc server.c unicode.c -o server -lm

host:
	./server

all: build

run:
	./calc
