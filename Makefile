client: client.c client.h
	gcc -Wall -Wno-unused-variable -g client.c -lpthread -o client

clean:
	rm client

