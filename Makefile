client: client.c client.h
	gcc -Wall -g client.c -lpthread -o client

clean:
	rm client

