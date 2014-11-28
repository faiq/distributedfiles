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
  int payloadSize = strlen (name) + 1;
  byte_buffer * send = malloc(sizeof(byte_buffer));
  printf("this is payloadSize %d\n", payloadSize);
  init_buf (payloadSize + 4, send); //add 4 to handle the int for size
  printf ("printing buffer %s\n", send->buffer);
  put_int (payloadSize, send); //send size
  printf ("printing buffer %s\n", send->buffer);
  put (OPEN, send);  
  put_string (name, send);
  printf ("printing buffer %s\n", send->buffer);
	 
	int n =  write (socketFd, send->buffer, strlen (send->buffer)); //write file name over socket
  if (n < 0 )  {
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 
	char buffer[256];
  memset (&buffer, 0, strlen (buffer));

  int k;
	k = read (socketFd, buffer, 255);

  if (k < 0 )  {
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 
  
  int ret = atoi (buffer); 
  return ret;
} 

int readFile (int fd, void * buffer) { 

}
