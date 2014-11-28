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

#define OPEN '0' 
#define READ '1'
#define WRITE '2' 
#define STAT '3' 
#define CLOSE '4' 

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
  init_buf (sizeof (char) * 5, &recv); 

  int k;
  k = read (socketFd, recv.buffer, 5);

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

  int n =  write (socketFd, send.buffer, strlen ((char *) send.buffer));//write file name over socket
  if (n < 0 )  {
    printf("error writing to socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 

  int k;
  char buf[1029]; //<size><id><payload>
  k = recv (socketFd, buf, 1029, 0); //recieve a maximum of 1029 4 size, 1 id, 1024 bytes

  if (k < 0 ) {
    printf("error reading from socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 

  //parse the size of bytes first
  char temp[sizeof(int)];
  memcpy (temp, buf, sizeof(int));
  int ret =  deserialize_int (temp);
  ret--; //subtract one extra byte for the id
  memcpy (buffer, buf + 5 * sizeof(char), sizeof(char) * ret); //this is the message it lives in the fifth space over copy it into buffer
  return ret; 
}
