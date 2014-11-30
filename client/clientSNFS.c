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
#include "clientSNFS.h"

#define OPEN 0 
#define READ 1
#define WRITE 2 
#define STAT 3 
#define CLOSE 4 

int socketFd; //global to save the file descriptor for the socket
struct sockaddr_in servAddr; //global to save address of the server

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


int socketFd; 
void setServer(char * serverIP, int port) {

  socketFd = socket (AF_INET, SOCK_STREAM, 0); //create a new socket connection

  if (socketFd < 0) { 
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  }
  
  memset (&servAddr, 0, sizeof (servAddr));
  servAddr.sin_addr.s_addr = htonl (host2IpAddr(serverIP));
  servAddr.sin_port = htons (port);
  servAddr.sin_family = AF_INET;

  int con = connect (socketFd, (struct sockaddr *)&servAddr, sizeof (servAddr));
  
  if (con < 0) {
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  }
  
}

int openFile (char * name) {  
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
  printf ("this is the size of the read response should be atleast 5 %d\n", k); 
  if (k < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 
  
  int j; 
  char buff;
  j = recv (socketFd, &buff, 1, 0); // send extra read for id to make the next read to be all our data (for ease)  
  printf ("this is the id of the read res %d\n", j); 
  if (j < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 
 
  int l;
  int size = deserialize_int(buf) - 1; //take out an extra byte for the id
  printf ("this is the size of the response %d\n", size);
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

  int n =  write (socketFd, send.buffer, sizeof (int) * 2 + sizeof (char) * (chars + 1)); //size <file><id><fd> 
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

int statFile (int fd, fileStat * buf) { 
  int payloadSize = sizeof(int) + sizeof(char); //msg = <size><id><fd>
  byte_buffer send; 

  init_buf (payloadSize + sizeof(int), &send);
  put_int (payloadSize, &send);
  put (STAT, &send);
  put_int (fd, &send);

  int n =  write (socketFd, send.buffer, sizeof (int) * 2 + sizeof (char)); //size <file><id><fd> 
  if (n < 0 )  {
    printf("error writing to socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  }

  int k;                                              /*gets the size of the response*/ 
  char resSize[4]; 
  k = recv (socketFd, resSize, 4, 0); 
  if (k < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 
  
  int j; 
  char resId; 
  j = recv (socketFd, &resId, 1, 0); 
  if (j < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 
  
  int l; 
  char statSize [4]; 
  l = recv (socketFd, statSize, 4, 0);
  if (l < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 

  int x; 
  char statCreate [4];
  x = recv (socketFd, statCreate, 4, 0); 
  if (x < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  }
  
  int y; 
  char statAccess [4]; 
  y = recv (socketFd, statAccess, 4, 0); 
  if (y < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  }

  int z; 
  char statMod [4]; 
  z = recv (socketFd, statMod, 4, 0); 
  if (z < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  }
 
  buf->file_size =  deserialize_int (statSize);
  buf->creation_time = deserialize_int (statCreate); 
  buf->access_time = deserialize_int (statAccess); 
  buf->mod_time = deserialize_int (statMod);
  
  printf("%d, %ld, %ld, %ld\n", buf->file_size, buf->creation_time, buf->access_time, buf->mod_time);

  return 1;
}
