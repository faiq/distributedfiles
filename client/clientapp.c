#include "clientSNFS.h" 
#include <stdio.h>  

int main (int argc, char ** argv)  {
  setServer("127.0.0.1", 8124);
  int x = openFile ("yo.txt");
	printf("this is x %d\n", x);
  char buff[1024];
  int y = readFile (x, buff);
  printf ("this is y %d\n", y);
  printf ("this is buff %s", buff);
}
