#include <sys/mman.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
//#include <unistd.h>
#include <stdbool.h>

typedef struct {
    uint8_t* data;
    uint64_t size;
    uint64_t cursor;
} buffer_t;

inline static void buf_write_8(buffer_t* buf, uint8_t val) {
    //printf("%02hx", val);
    (buf)->data[(buf)->cursor++] = (val & 0xff);
}

inline static void buf_write_16(buffer_t* buf, uint16_t val) {
    buf_write_8(buf, val);
    buf_write_8(buf, (val) >> 8);
}

inline static void buf_write_32(buffer_t* buf, uint32_t val) {
    buf_write_16(buf, val);
    buf_write_16(buf, (val) >> 16);
}

inline static void buf_write_64(buffer_t* buf, uint64_t val) {
    buf_write_32(buf, val);
    buf_write_32(buf, (val) >> 32);
}

static inline buffer_t alloc_buf(uint64_t size) {
    uint8_t* data = mmap(
        0, size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1, 0
    );
    return (buffer_t) {
        .data = data,
        .size = size
    };
}

static inline void buf_hexdump(buffer_t buf) {
    for (int i = 0; i < buf.size; i += 16) {
        uint8_t row_nonzero = 0;
        for (int j = 0; j < 16; j++) {
            row_nonzero |= buf.data[i+j];
        }
        if (row_nonzero) {
            printf("%04x:", i);
            for (int j = 0; j < 16; j++) {
                if (j % 4 == 0) {
                    printf(" ");
                }
                printf("%02x", buf.data[i+j] & 0xff);
            }
            printf("\n");
        }
    }
}

//static inline void buf_print(buffer_t buf) {
//    int stdout_fd = 1;
//    write(stdout_fd, buf.data, buf.size);
//}
