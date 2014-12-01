#include "clientSNFS.h" 
#include <stdio.h>  
#include <stdlib.h>
#include <string.h> 

void revstr(char * str) {
  int right = strlen(str) - 1;
  int left = 0;
  while (left < right) {
    char c = str[right];
    str[right] = str[left];
    str[left] = c;
    ++left;
    --right;
  }
}

int main (int argc, char ** argv)  {
  if (argc <  4) { 
    printf ("not enough arguments\n");
    exit (1);
  }

  char * server = argv[1];
  int port = atoi(argv[2]);
  char * file = argv[3]; 
  
  setServer(server, port);  
  int infd = openFile ("file.in");
  int filefd = openFile(file); 
  fileStat stat; 
  statFile (filefd, &stat); 
  char * buffer = malloc (stat.file_size); 
  readFile (filefd, buffer); 
  writeFile (infd, buffer); 
  statFile (infd, &stat);
  int rfd = openFile ("reverse.in");
  readFile (infd, buffer); 
  revstr (buffer); 
  writeFile (rfd, buffer);
  statFile (rfd, &stat);   

  closeFile(rfd);
  closeFile(filefd);
  closeFile (infd);

  free (buffer);
  return 0;
}
