#include <errno.h>
#include <error.h>
#include <stdbool.h>
#include <stdint.h>   // uint32_t
#include <stdio.h>    // printf, stderr
#include <stdlib.h>   // size_t
#include <string.h>   // memcpy
#include <sys/mman.h> // mmap

typedef bool (*func_t)();

func_t is_valid_utf8 = NULL;

size_t const MAPPED_SIZE = 2048;

static char const utf8d[] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

        0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
        12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
        12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
        12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
        12,36,12,12,12,12,12,12,12,12,12,12,
};

static void const *table_pointer = &utf8d;

void init_function(char * const *str) {
    static char binary[] = {
            0x48, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 2-9
            0x48, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 12-19
            0x31,0xc9,
            0x8a,0x17,
            0x31,0xc0,
            0x84,0xd2,
            0x74,0x22,
            0x0f,0xb6,0xd2,
            0x44,0x0f,0xbe,0x04,0x32,
            0x42,0x8d,0x8c,0x01,0x00,0x01,0x00,0x00,
            0x0f,0xbe,0x0c,0x31,
            0x83,0xf9,0x0c,
            0x74,0x09,
            0x48,0xff,0xc7,
            0x8a,0x17,
            0x84,0xd2,
            0x75,0xde,
            0x85,0xc9,
            0x0f,0x94,0xc0,
            0xc3,
            0x66,0x2e,0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00,
            0x0f,0x1f,0x40,0x00,
    };
    memcpy(&binary[2], str, 8);
    memcpy(&binary[12], &table_pointer, 8);

    void *memory = mmap(0, MAPPED_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        error(EXIT_FAILURE, errno, "Can't mmap memory");
    }
    memcpy(memory, binary, sizeof(binary));
    if (mprotect(memory, MAPPED_SIZE, PROT_EXEC | PROT_WRITE)) {
        error(EXIT_FAILURE, errno, "Can't adjust permissions");
    }
    is_valid_utf8 = memory;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s string\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *string = argv[1];
    init_function(&string);
    if (is_valid_utf8()) {
        printf("valid\n");
    } else {
        printf("invalid\n");
    }
    if (munmap(is_valid_utf8, MAPPED_SIZE)) {
        error(EXIT_FAILURE, errno, "Can't unmap");
    }
}

