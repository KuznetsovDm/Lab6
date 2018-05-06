CC=gcc

all: sharedClient sharedServer

sharedClient : client.o lib_d.so
	$(CC) -pthread client.o -o sharedClient -L./ -l_d

sharedServer : server.o lib_d.so
	$(CC) -pthread server.o -o sharedServer -L./ -l_d

client.o : client.c
	$(CC) -c client.c -o client.o

server.o : server.c
	$(CC) -c server.c -o server.o

lib_d.so : MultModulo.o
	$(CC) -shared MultModulo.o -o ./lib_d.so

MultModulo.o : MultModulo.c
	$(CC) -c -fPIC MultModulo.c -o MultModulo.o

clean :
	rm sharedClient client.o lib_d.so MultModulo.o sharedServer server.o
