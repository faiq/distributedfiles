#ifndef SERIAL_H
#define SERIAL_H
typedef struct byte_buffer {
    int offset;
    void* buffer;
};
void put_int(int, byte_buffer*);
void put_string(char*, byte_buffer*);
void put_bytes(void*, size_t, byte_buffer*);
void put(int, byte_buffer*);
#endif
