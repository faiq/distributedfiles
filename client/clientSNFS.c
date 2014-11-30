#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <netinet/ip.h>
#include <sys/types.h>
#include <errno.h>
#include "serialize.h"

#define OPEN 0 
#define READ 1
#define WRITE 2 
#define STAT 3 
#define CLOSE 4 

int socketFd; //global to save the file descriptor for the socket
struct sockaddr_in servAddr; //global to save address of the server
int fds[256];
int total_open;

static int host2IpAddr(char *anIpName){

  struct hostent *hostEntry;
  struct in_addr *scratch;
 
  if ((hostEntry = gethostbyname ( anIpName )) == (struct hostent*) NULL){ 
    printf("error gettig host from the provided string, error%d\n",errno);
    perror("meaning:"); exit(0);
  }

  scratch = (struct in_addr *) hostEntry->h_addr;
 
  return (ntohl(scratch->s_addr));
   
}

void setServer(char * serverIP, int port) {
  bzero(fds, 256);
  total_open = 0;

  socketFd = socket (AF_INET, SOCK_STREAM, 0); //create a new socket connection

  if (socketFd < 0) { 
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  }
  
  memset (&servAddr, 0, sizeof (servAddr));
  servAddr.sin_addr.s_addr = htonl (host2IpAddr(serverIP));
  servAddr.sin_port = htons (port);
  servAddr.sin_family = AF_INET;
}

int openFile (char * name) {  
  if (total_open == 0 && (con = connect(socketFd, (struct sockaddr*)&servAddr, sizeof(servAddr))) < 0) {
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  }

  int payloadSize =  sizeof(char) * strlen (name) + sizeof(char); //we send name + 1 byte for id
  byte_buffer send; 

  init_buf (payloadSize + sizeof(int), &send); //add 4 to handle the int for size
  put_int (payloadSize, &send); //send size
  put (OPEN, &send);
  put_string (name, &send);

	int n =  write (socketFd, send.buffer, payloadSize + sizeof(int)); //write file name over socket
  if (n < 0 )  {
    printf("error writing in socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 
  byte_buffer recv; 
  init_buf (sizeof (char) * 9, &recv); 

  int k;
  k = read (socketFd, recv.buffer, 9);
  
  if (k < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 
  
  char * buffer = malloc(sizeof (int));
  memcpy (buffer, &recv.buffer[5], sizeof(int));
  int ret = deserialize_int(buffer);
  free(buffer);
  int i;
  for (i = 0; i < 256; i++) {
    if (fds[i] == 0) {
      fds[i] = ret;
      total_open++;
      break;
    }
  }
  return ret;
} 

int readFile (int fd, void * buffer) { 
  int payloadSize = sizeof(int) + sizeof(char); //msg = <size><id><fd>
  byte_buffer send; 
  
  init_buf (payloadSize + sizeof(int), &send); //add the extra int bc we dont include size in paylad
  put_int (payloadSize, &send);
  put (READ, &send);
  put_int (fd, &send);

  int n =  write (socketFd, send.buffer, 2 * sizeof(int) + sizeof(char));//write file name over socket
  if (n < 0 )  {
    printf("error writing to socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 

  int k;
  char buf[4]; 
  k = recv (socketFd, buf, 4, 0); 

  if (k < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 
  
  int j; 
  char buff;
  j = recv (socketFd, &buff, 1, 0); // send extra read for id to make the next read to be all our data (for ease)  

  if (j < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 
 
  int l;
  int size = deserialize_int(buf) - 1; //take out an extra byte for the id
  char buffr[size]; 
  l = recv (socketFd, buffr, size, 0);
  
  if (l < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 

  memcpy (buffer, buffr, size); 
  return size;  
}

int writeFile(int fd, void * buffer) { 
  int chars = (int) strlen ( (char *) buffer);
  int payloadSize = sizeof (int) + chars * sizeof (char) + sizeof (char); //pass fd, char, contents
  byte_buffer send; 
  
  init_buf (payloadSize + sizeof (int), &send); 
  put_int (payloadSize, &send); //sizefirst
  put (WRITE, &send); 
  put_int (fd, &send); 
  put_string ((char *) buffer, &send);

  int n =  write (socketFd, send.buffer, 4 + 4 + 1 + chars); //size <file><id><fd> 
  printf ("this is n %d\n", n);
  if (n < 0 )  {
    printf("error writing to socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 

  int k;
  char buf[5]; 
  k = read (socketFd, buf, 5); 
  if (k < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 

  int size = deserialize_int(&buf[1]) - 1; //take out an extra byte for the id
  return size;
}

int closeFile(int fd) {
    int i;
    for (i = 0; i < 256; i++) {
        if (fds[i] == fd) {
            fds[i] = 0;
            total_open--;
            break;
        }
    }
    byte_buffer send;
    init_buf(9, &send);
    put_int(5, &send);
    put(4, &send);
    put_int(fd, &send);
    write(socketFd, send.buffer, 9);
    char size_buf[4];
    recv(socketFd, size_buf, 4, 0);
    int size = deserialize_int(size_buf);
    char* buffer = malloc(size);
    recv(socketFd, buffer, size, 0);
    int response = deserialize_int(&buffer[1]);
    free(send.buffer);
    if (total_open == 0) {
        init_buf(5, &send);
        put_int(1, &send);
        put(5, &send);
        write(socketFd, send.buffer, 5);
        free(send.buffer);
        close(socketFd);
    }
    return response;
}
