#ifndef CLIENT_H
#define CLIENT_H
#include <time.h>
typedef struct fileStat {
  int file_size;
  time_t creation_time; 
  time_t access_time;
  time_t mod_time;
} fileStat; 
void setServer(char *, int);
int openFile(char *);
int readFile(int, void *); 
int writeFile(int, void *);
int statFile(int, fileStat *);  
int closeFile(int);
#endif
