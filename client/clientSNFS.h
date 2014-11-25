#ifndef HEADER_FILE
#define HEADER_FILE
#include <time.h>
void setServer(char *, int);
int openFile(char *);
int readFile(int, void *); 
int statFile(int, fileStat *);  
int closeFile(int);
typedef struct fileStat {
  int file_size;
  time_t creation_time; 
  time_t access_time;
  time_t mod_time;
} fileStat; 
#endif
