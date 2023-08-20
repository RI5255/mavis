#include "buffer.h"

Buffer *newBuffer(uint8_t *p, int len) {
    Buffer *buf = malloc(sizeof(Buffer));
    assert(buf != NULL);

    *buf = (Buffer){
        .cursor = 0, 
        .len = len,
        .p = p
    };

    return buf;
}

uint8_t readByte(Buffer *buf) {
    if(buf->cursor + 1 > buf->len)
        return 0;

    return buf->p[buf->cursor++];
}

uint32_t readWord(Buffer *buf) {
    if(buf->cursor + 4 > buf->len)
        return 0;
    
    uint32_t r = *(uint32_t *)&buf->p[buf->cursor];
    buf->cursor += 4;
    return r;
}

// LEB128(Little Endian Base 128)
uint32_t readU32(Buffer *buf) {
    uint32_t result = 0, shift = 0;
    while(1) {
        uint8_t byte = readByte(buf); 
        result |= (byte & 0b1111111) << shift;
        shift += 7;
        if((0b10000000 & byte) == 0)
            return result;
    }
}

int32_t readI32(Buffer *buf) {
    int32_t result = 0, shift = 0;
    while(1) {
        uint8_t byte = readByte(buf);
        result |= (byte & 0b1111111) << shift;
        shift += 7;
        if((0b10000000 & byte) == 0) {
            if((byte & 0b1000000) != 0)
                return result |= ~0 << shift;
            else
                return result;
        }
    }
}

char * readName(Buffer *buf) {
    uint32_t n = readU32(buf);
    char *name = malloc(sizeof(char) * (n + 1));

    for(uint32_t i = 0; i < n; i++) {
        name[i] = readByte(buf);
    }
    name[n] = '\0';
    return name;
}

Buffer * readBuffer(Buffer *buf, int len) {
    if(buf->cursor + len > buf->len)
        return NULL;

    Buffer *new = newBuffer(buf->p + buf->cursor, len);
    buf->cursor += len;
    return new;
}

bool eof(Buffer *buf) {
    return buf->cursor == buf->len;
}