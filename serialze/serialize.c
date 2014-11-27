#include <string.h>
#include <arpa/inet.h>
#include "serialize.h"

void init_buf(size_t size, byte_buffer* buffer) {
    buffer->offset = 0;
    buffer->buffer = malloc(size);
}

void put_int(int val, byte_buffer* buffer) {
    char* buf = (char*) buffer->buffer;
    int off = buffer->offset;
    uint32_t sending = htonl(val);
    buf[0+off] = sending >> 24;
    buf[1+off] = sending >> 16;
    buf[2+off] = sending >> 8;
    buf[3+off] = sending; 
    buffer->offset += 4;
}

void put_string(char* str, byte_buffer* buffer) {
    put_bytes(str, strlen(str), buffer);
}
void put_bytes(void* val, size_t size, byte_buffer* buffer) {
    int i;
    int off = buffer->offset;
    char* buf = (char*)buffer->buffer;
    for (i = off; i < size + off; i++) {
        buf[i] = str[i];
    }
    buffer->offset += size;
}

void put(int byte, byte_buffer* buffer) {
    char* buf = (char*)buffer->buffer;
    buf[buffer->offset] = (char)byte;
    buffer->offset++;
}
