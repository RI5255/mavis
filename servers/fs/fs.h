#pragma once

#include <stdint.h>

struct fs_header {
    uint16_t    header_size;
    uint16_t    num_files;
    uint32_t    padding;
}__attribute__((packed));

struct fs_entry {
    char        name[16];
    uint32_t    offset;
    uint32_t    len;
} __attribute__((packed));

#define CLOSE           0
#define OPEN            1
#define NUM_MAX_FILE    3

struct file {
    int state;
    struct fs_entry *entry;
};