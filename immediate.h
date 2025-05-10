#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uint64_t data;
    uint8_t size;
} immediate_t;

#define immediate_data(imm) ((imm).data)
#define immediate_size(imm) ((imm).size)

inline static void print_imm(immediate_t imm) {
    switch (immediate_size(imm)) {
    case ARG_SIZE_8:
        uint8_t data8 = immediate_data(imm);
        printf("imm_8(%hhd / %hhu / 0x%02hhx)", data8, data8, data8);
        break;
    case ARG_SIZE_16:
        uint16_t data16 = immediate_data(imm);
        printf("imm_16(%hd / %hu / 0x%04hx)", data16, data16, data16);
        break;
    case ARG_SIZE_32:
        uint32_t data32 = immediate_data(imm);
        printf("imm_32(%d / %u / 0x%08x)", data32, data32, data32);
        break;
    case ARG_SIZE_64:
        uint64_t data64 = immediate_data(imm);
        printf("imm_64(%ld / %lu / 0x%016lx)", data64, data64, data64);
        break;
    default:
        printf("imm(BAD)");
        break;
    }
}
