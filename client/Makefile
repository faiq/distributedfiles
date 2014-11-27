# Makefile for executable adjust

# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -g

# ****************************************************
# Entries to bring the executable up to date

client-app: client.o serialize.o
	$(CC) $(CFLAGS) -o client-app client.o serialize.o

server-app: server.o serialize.o
	$(CC) $(CFLAGS) -o server-app server.o serialize.o

client.o: client/clientSNFS.c client/clientSNFS.h
	$(CC) $(CFLAGS) -Iclient -Iserialize -c client/clientSNFS.c -o client.o

server.o: server/server.c server/server.h
	$(CC) $(CFLAGS) -Iserver -Iserialize -c server/server.c -o server.o

serialize.o: serialize/serialize.c serialize/serialize.h
	$(CC) $(CFLAGS) -Iserialize -c serialize/serialize.c
