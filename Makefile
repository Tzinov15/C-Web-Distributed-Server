#-Wno-unused-variable
all: client server

client: client.c client.h
	
	gcc -lssl -lcrypto -Wall  -Wno-deprecated-declarations -g client.c -lpthread -o client

server: server.c server.h
	gcc -Wall server.c -lpthread -o server
clean:
	rm client
	rm server

