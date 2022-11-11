
all: client server

client: client.c
	gcc -Wall client.c -o client -lrt

server: server.c
	gcc -Wall server.c -o server -lrt

clean:
	rm -f client server