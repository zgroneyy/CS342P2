all: server client
server: server.c
		gcc -g server.c -o server -lpthread -lrt 
client: client.c
		gcc -g client.c -o client -lrt
