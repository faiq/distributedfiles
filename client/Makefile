# Makefile for executable adjust

# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -ansi -pedantic -Wall -g

# ****************************************************
# Entries to bring the executable up to date

app: client.o serialize.o
	$(CC) $(CFLAGS) -o app client.o serialize.o

client.o: clientSNFS.c clientSNFS.h
	$(CC) $(CFLAGS) -c clientSNFS.c 

serialize.o: serialize.c serialize.h
	$(CC) $(CFLAGS) -c ../serialize/serialize.c
