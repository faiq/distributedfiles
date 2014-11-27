#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h> 
#include <netinet/ip.h>
#include <sys/types.h>
#include <errno.h>

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
    perror("meaning:"); exit(0);
  }

  scratch = (struct in_addr *) hostEntry->h_addr;
 
  return (ntohl(scratch->s_addr));
   
}

void setServer(char * serverIP, int port) {

  socketFd = socket (AF_INET, SOCK_STREAM, 0); //create a new socket connection

  if (socketFd < 0) { 
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  }
  
  memset (&servAddr, 0, sizeof (servAddr));
  servAddr.sin_addr.s_addr = htonl (host2IpAddr(serverIP));
  servAddr.sin_port = htons (port);
  servAddr.sin_family = AF_INET;

  int con = connect (socketFd, (struct sockaddr *)&servAddr, sizeof (servAddr));
  
  if (con < 0) {
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  }
  
}

int openFile (char * name) {  
	int payloadSize = strlen (name);
  unsigned char buffer[payloadSize + 1 + 4]; //payloadSize + ID + INT for length
	//first encode size into buffer 
	int t = payloadSize + 1;
	serializeInt (buffer, t); // total size of the message going along
	buffer[4] = OPEN;
	printf ("printing buffer %s\n", buffer);
  strncpy (&buffer[5], name, strlen (buffer)); // copy message to the next part of the buffer we send
	printf ("printing buffer %s\n", buffer);

  int n =  write (socketFd, buffer, strlen (buffer)); //write file name over socket
  if (n < 0 )  {
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 

  memset (&buffer, 0, strlen (buffer));

  int k;
	k = read (socketFd, buffer, 255);

  if (k < 0 )  {
    printf("error creating client socket, error%d\n",errno);
    perror("meaning:"); exit(0);
  } 
  
  int ret = atoi (buffer); 
  return ret;
} 

int readFile (int fd, void * buffer) { 

}
