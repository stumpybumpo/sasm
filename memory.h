#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uint64_t disp;
    register_t base;
    register_t index;
    uint8_t scale;
    uint8_t disp_size;
    uint8_t size;
} memory_t;

#define MEMORY_SCALE_1 0b00
#define MEMORY_SCALE_2 0b01
#define MEMORY_SCALE_4 0b10
#define MEMORY_SCALE_8 0b11

#define memory_base(mem) ((mem).base)
#define memory_index(mem) ((mem).index)
#define memory_scale(mem) ((mem).scale)
#define memory_disp(mem) ((mem).disp)
#define memory_disp_size(mem) ((mem).disp_size)
#define memory_has_disp(mem) ((mem).disp_size != ARG_SIZE_NONE)
#define memory_size(mem) ((mem).size)

inline static void print_mem(memory_t mem) {
    if (memory_size(mem) != ARG_SIZE_8  &&
        memory_size(mem) != ARG_SIZE_16 &&
        memory_size(mem) != ARG_SIZE_32 &&
        memory_size(mem) != ARG_SIZE_64) {
        printf("mem(BAD)");
        return;
    }
    register_t base = memory_base(mem);
    register_t index = memory_index(mem);
    printf("mem(");
    print_reg(base);

    if (!register_is_none(index)) {
        switch (memory_scale(mem)) {
        case MEMORY_SCALE_1:
            printf(" + 1 * ");
            print_reg(index);
            break;
        case MEMORY_SCALE_2:
            printf(" + 2 * ");
            print_reg(index);
            break;
        case MEMORY_SCALE_4:
            printf(" + 4 * ");
            print_reg(index);
            break;
        case MEMORY_SCALE_8:
            printf(" + 8 * ");
            print_reg(index);
            break;
        default:
            printf(" + BAD");
            break;
        }
    }

    switch (memory_disp_size(mem)) {
    case ARG_SIZE_8:
        uint8_t disp8 = memory_disp(mem);
        printf(" + (%hhd / %hhu / 0x%02hhx))", disp8, disp8, disp8);
        break;
    case ARG_SIZE_16:
        uint16_t disp16 = memory_disp(mem);
        printf(" + (%hd / %hu / 0x%04hx))", disp16, disp16, disp16);
        break;
    case ARG_SIZE_32:
        uint32_t disp32 = memory_disp(mem);
        printf(" + (%d / %u / 0x%08x))", disp32, disp32, disp32);
        break;
    case ARG_SIZE_64:
        uint64_t disp64 = memory_disp(mem);
        printf(" + (%ld / %lu / 0x%016lx))", disp64, disp64, disp64);
        break;
    }
    printf(")");
}
