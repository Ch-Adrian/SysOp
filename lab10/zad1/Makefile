

all: server client

server:
	gcc -Wall -fsanitize=address server.c -o server -pthread
# 0
client:
	gcc -Wall -fsanitize=address client.c -o client

clean:
	rm -f server client