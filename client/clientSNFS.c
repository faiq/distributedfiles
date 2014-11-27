#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h> 
#include <netinet/ip.h>
#include <sys/types.h>


int socketFd; 
void setServer(char * serverIP, int port) {
  struct hostnet * hostEntry; 
  if ((hostEntry = gethostbyname ( serverIP )) == (struct hostent*) NULL)
    return 0;
  
}
