#ifndef SERVER_H
#define SERVER_H
#include <time.h>
typedef struct fileStat {
  int file_size;
  time_t creation_time; 
  time_t access_time;
  time_t mod_time;
} fileStat; 
#endif
