#include "buffer.h"
#include "memory.h"
#include <stdint.h>

struct buffer *newbuffer(uint8_t *p, int len) {
    struct buffer *buf = malloc(sizeof(struct buffer));

    *buf = (struct buffer){
        .cursor = 0, 
        .len = len,
        .p = p
    };

    return buf;
}

struct buffer *newstack(uint8_t *p, int len) {
    struct buffer *stack = malloc(sizeof(struct buffer));

    *stack = (struct buffer){
        .cursor = len, 
        .len = len,
        .p = p
    };

    return stack;
}

uint8_t readbyte(struct buffer *buf) {
    if(buf->cursor + 1 > buf->len)
        return 0;

    return buf->p[buf->cursor++];
}

uint32_t readu32(struct buffer *buf) {
    if(buf->cursor + 4 > buf->len)
        return 0;
    
    uint32_t r = *(uint32_t *)&buf->p[buf->cursor];
    buf->cursor += 4;
    return r;
}

int32_t readi32(struct buffer *buf) {
    if(buf->cursor + 4 > buf->len)
        return 0;
    
    int32_t r = *(int32_t *)&buf->p[buf->cursor];
    buf->cursor += 4;
    return r;
}

int64_t readi64(struct buffer *buf) {
    if(buf->cursor + 8 > buf->len)
        return 0;
    
    int64_t r = *(int64_t *)&buf->p[buf->cursor];
    buf->cursor += 8;
    return r;
}

// LEB128(Little Endian Base 128)
uint64_t readu64_LEB128(struct buffer *buf) {
    uint64_t result = 0, shift = 0;
    while(1) {
        uint8_t byte = readbyte(buf); 
        result |= (byte & 0b1111111) << shift;
        shift += 7;
        if((0b10000000 & byte) == 0)
            return result;
    }
}

int64_t readi64_LEB128(struct buffer *buf) {
    int64_t result = 0, shift = 0;
    while(1) {
        uint8_t byte = readbyte(buf);
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

char * readname(struct buffer *buf) {
    uint32_t n = readu64_LEB128(buf);
    char *name = malloc(sizeof(char) * (n + 1));

    for(uint32_t i = 0; i < n; i++) {
        name[i] = readbyte(buf);
    }
    name[n] = '\0';
    return name;
}

struct buffer * readbuffer(struct buffer *buf, int len) {
    if(buf->cursor + len > buf->len)
        return NULL;

    struct buffer *new = newbuffer(buf->p + buf->cursor, len);
    buf->cursor += len;
    return new;
}

bool eof(struct buffer *buf) {
    return buf->cursor == buf->len;
}

// We no longer need to use LSB128 at runtime.
uint8_t wirtebyte(struct buffer *buf, uint8_t val) {
    if(buf->cursor - 1 < 0)
        return 0;

    buf->p[--buf->cursor] = val;
    return val;
}

uint32_t writeu32(struct buffer *buf, uint32_t val) {
    if(buf->cursor - 4 < 0)
        return 0;

    buf->cursor -= 4;
    *(uint32_t *)&buf->p[buf->cursor] = val;
    return val;
}

int32_t writei32(struct buffer *buf, int32_t val) {
    if(buf->cursor - 4 < 0)
        return 0;

    buf->cursor -= 4;
    *(int32_t *)&buf->p[buf->cursor] = val;
    return val;
}

int64_t writei64(struct buffer *buf, int64_t val) {
    if(buf->cursor - 8 < 0)
        return 0;

    buf->cursor -= 8;
    *(int64_t *)&buf->p[buf->cursor] = val;
    return val;
}

uint8_t storebyte(struct buffer *buf, int32_t ea, uint8_t val) {
    if(ea + 1 > buf->len)
        return 0;

    *(buf->p + ea) = val;
    return val;
}

int32_t storei32(struct buffer *buf, int32_t ea, int32_t val) {
    if(ea + 4 > buf->len)
        return 0;
    
    *(int32_t *)(buf->p + ea) = val;
    return val;
}

int64_t storei64(struct buffer *buf, int32_t ea, int64_t val) {
    if(ea + 8 > buf->len)
        return 0;
    
    *(int64_t *)(buf->p + ea) = val;
    return val;
}

uint8_t loadbyte(struct buffer *buf, int32_t ea) {
    if(ea + 1 > buf->len)
        return 0;
    
    return (buf->p + ea)[0];
}

int32_t loadi32(struct buffer *buf, int32_t ea) {
    if(ea + 4 > buf->len)
        return 0;
    
    return *(int32_t *)(buf->p + ea);
}

int64_t loadi64(struct buffer *buf, int32_t ea) {
    if(ea + 8 > buf->len)
        return 0;
    
    return *(int64_t *)(buf->p + ea);
}