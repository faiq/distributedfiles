#include "clientSNFS.h" 
 
int main (int argc, char ** argv)  {
  setServer("127.0.0.1", 8124);
  int x = openFile ("yo.txt"); 
}
