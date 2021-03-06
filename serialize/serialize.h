#ifndef SERIAL_H
#define SERIAL_H
typedef struct byte_buffer {
    int offset;
    char* buffer;
} byte_buffer;
void init_buf(size_t, byte_buffer*);
void put_int(int, byte_buffer*);
void put_string(char*, byte_buffer*);
void put_bytes(void*, size_t, byte_buffer*);
void put(int, byte_buffer*);
int deserialize_int(char*);
#endif
