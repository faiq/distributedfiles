#include "clientSNFS.h" 
#include <stdio.h>  

int main (int argc, char ** argv)  {
  setServer("127.0.0.1", 8124);
  int x = openFile ("yo.txt");
	printf("this is x %d\n", x);
  char buff[1024];
  int readRes = readFile (x, buff);
  printf ("this is readRes %d\n", readRes);
  printf ("this is buff %s", buff);
  int z = writeFile (x, "How can I find out the size of a file? I opened with an application written in C. I would like to know the size");
  fileStat stuff; 
  int a = statFile(x, &stuff);
  return 0;
}
