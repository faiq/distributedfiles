#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "serialize.h"
#include <stdio.h> 

void init_buf(size_t size, byte_buffer* buffer) {
    buffer->offset = 0;
    buffer->buffer = malloc(size);
    memset(buffer->buffer,0,size);
}

void put_int(int val, byte_buffer* buffer) {
    val = htonl(val);
    memcpy(buffer->buffer + buffer->offset,&val,sizeof(unsigned int));
    buffer->offset += 4;
}

void put_string(char* str, byte_buffer* buffer) {
    memcpy (buffer->buffer + buffer->offset, str, strlen (str));
    buffer->offset += strlen (str);
}

//we don't really need this function anymore 
void put_bytes(void* val, size_t size, byte_buffer* buffer) {
    int i;
    char* str = (char*)val;
    for (i = 0; i < size; i++) {
        buffer->buffer[i+buffer->offset] = str[i];
    }
    buffer->offset += size;
}

void put(int byte, byte_buffer* buffer) {
    buffer->buffer[buffer->offset] = (char)byte;
    buffer->offset++;
}

int deserialize_int(char * buffer) {
    int ret = 0;
    ret += buffer[0] << 24;
    ret += buffer[1] << 16;
    ret += buffer[2] << 8;
    ret += buffer[3];
    return ret;
}
