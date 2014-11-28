#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "serialize.h"
#include <stdio.h> 

void init_buf(size_t size, byte_buffer* buffer) {
    buffer->offset = 0;
    printf("offset, set\n"); 
    buffer->buffer = malloc(size);
    printf("buffer set \n");
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
    char* str = (char*)val;
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

int deserialize_int(void* buffer) {
    char* buf = (char*)buffer;
    int ret = 0;
    ret += buf[0] << 24;
    ret += buf[1] << 16;
    ret += buf[2] << 8;
    ret += buf[3];
    return ret;
}
