#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uint8_t id : 4;
    uint8_t type : 4;
} register_t;

#define REGISTER_TYPE_NONE        0
#define REGISTER_TYPE_8_BIT       1
#define REGISTER_TYPE_16_BIT      2
#define REGISTER_TYPE_32_BIT      3
#define REGISTER_TYPE_64_BIT      4
#define REGISTER_TYPE_8_BIT_REX   5
#define REGISTER_TYPE_32_BIT_IP   6
#define REGISTER_TYPE_64_BIT_IP   7
#define REGISTER_TYPE_X87         8
#define REGISTER_TYPE_MMX         9
#define REGISTER_TYPE_XMM        10
#define REGISTER_TYPE_YMM        11
#define REGISTER_TYPE_SEGMENT    12
#define REGISTER_TYPE_CONTROL    13
#define REGISTER_TYPE_DEBUG      14

register_t REGISTER_NONE = {.type = REGISTER_TYPE_NONE};

#define register_id(reg) ((reg).id)
#define register_type(reg) ((reg).type)
#define register_id_low(reg) (register_id(reg) & 0b0111)
#define register_id_high(reg) (register_id(reg) & 0b1000)
#define register_is_none(reg) (register_type(reg) == REGISTER_TYPE_NONE)
#define register_is_8(reg) (register_type(reg) == REGISTER_TYPE_8_BIT)
#define register_is_16(reg) (register_type(reg) == REGISTER_TYPE_16_BIT)
#define register_is_32(reg) (register_type(reg) == REGISTER_TYPE_32_BIT)
#define register_is_64(reg) (register_type(reg) == REGISTER_TYPE_64_BIT)
#define register_is_8_rex(reg) (register_type(reg) == REGISTER_TYPE_8_BIT_REX)
#define register_is_32_ip(reg) (register_type(reg) == REGISTER_TYPE_32_BIT_IP)
#define register_is_64_ip(reg) (register_type(reg) == REGISTER_TYPE_64_BIT_IP)
#define register_is_x87(reg) (register_type(reg) == REGISTER_TYPE_X87)
#define register_is_mmx(reg) (register_type(reg) == REGISTER_TYPE_MMX)
#define register_is_xmm(reg) (register_type(reg) == REGISTER_TYPE_XMM)
#define register_is_ymm(reg) (register_type(reg) == REGISTER_TYPE_YMM)
#define register_is_segment(reg) (register_type(reg) == REGISTER_TYPE_SEGMENT)
#define register_is_control(reg) (register_type(reg) == REGISTER_TYPE_CONTROL)
#define register_is_debug(reg) (register_type(reg) == REGISTER_TYPE_DEBUG)

#define register_is_general(reg) ( \
    register_is_8(reg)  || \
    register_is_16(reg) || \
    register_is_32(reg) || \
    register_is_64(reg) || \
    register_is_8_rex(reg) \
)

#define register_is_ip(reg) ( \
    register_is_32_ip(reg) || \
    register_is_64_ip(reg)    \
)

#define register_is_8_no_rex(reg) ( \
    register_is_8(reg) && \
    (4 <= register_id(reg)) && \
    (register_id(reg) < 8) \
)

inline static uint8_t register_size_general(register_t reg) {
    switch (register_type(reg)) {
    case REGISTER_TYPE_8_BIT:
    case REGISTER_TYPE_8_BIT_REX:
        return ARG_SIZE_8;
    case REGISTER_TYPE_16_BIT:
        return ARG_SIZE_16;
    case REGISTER_TYPE_32_BIT:
    case REGISTER_TYPE_32_BIT_IP:
        return ARG_SIZE_32;
    case REGISTER_TYPE_64_BIT:
    case REGISTER_TYPE_64_BIT_IP:
        return ARG_SIZE_64;
    default:
        return ARG_SIZE_NONE;
    }
}

inline static uint8_t register_size(register_t reg) {
    switch (register_type(reg)) {
    case REGISTER_TYPE_8_BIT:
    case REGISTER_TYPE_8_BIT_REX:
        return ARG_SIZE_8;
    case REGISTER_TYPE_16_BIT:
    case REGISTER_TYPE_SEGMENT:
        return ARG_SIZE_16;
    case REGISTER_TYPE_32_BIT:
    case REGISTER_TYPE_32_BIT_IP:
    case REGISTER_TYPE_CONTROL:
    case REGISTER_TYPE_DEBUG:
        return ARG_SIZE_32;
    case REGISTER_TYPE_64_BIT:
    case REGISTER_TYPE_64_BIT_IP:
    case REGISTER_TYPE_MMX:
        return ARG_SIZE_64;
    case REGISTER_TYPE_XMM:
        return ARG_SIZE_128;
    case REGISTER_TYPE_YMM:
        return ARG_SIZE_256;
    default:
        return ARG_SIZE_NONE;
    }
}

inline static void print_reg(register_t reg) {
    int reg_id = register_id(reg);

    switch (register_type(reg)) {
    case REGISTER_TYPE_NONE:
        printf("reg(NONE)");
        break;
    case REGISTER_TYPE_8_BIT:
        printf("reg(8_BIT, %d)", reg_id);
        break;
    case REGISTER_TYPE_16_BIT:
        printf("reg(16_BIT, %d)", reg_id);
        break;
    case REGISTER_TYPE_32_BIT:
        printf("reg(32_BIT, %d)", reg_id);
        break;
    case REGISTER_TYPE_64_BIT:
        printf("reg(64_BIT, %d)", reg_id);
        break;
    case REGISTER_TYPE_8_BIT_REX:
        printf("reg(8_BIT_REX, %d)", reg_id);
        break;
    case REGISTER_TYPE_32_BIT_IP:
        printf("reg(32_BIT_IP, %d)", reg_id);
        break;
    case REGISTER_TYPE_64_BIT_IP:
        printf("reg(64_BIT_IP, %d)", reg_id);
        break;
    case REGISTER_TYPE_X87:
        printf("reg(X87, %d)", reg_id);
        break;
    case REGISTER_TYPE_MMX:
        printf("reg(MMX, %d)", reg_id);
        break;
    case REGISTER_TYPE_XMM:
        printf("reg(XMM, %d)", reg_id);
        break;
    case REGISTER_TYPE_YMM:
        printf("reg(YMM, %d)", reg_id);
        break;
    case REGISTER_TYPE_SEGMENT:
        printf("reg(SEGMENT, %d)", reg_id);
        break;
    case REGISTER_TYPE_CONTROL:
        printf("reg(CONTROL, %d)", reg_id);
        break;
    case REGISTER_TYPE_DEBUG:
        printf("reg(DEBUG, %d)", reg_id);
        break;
    default:
        printf("reg(BAD)");
        break;
    }
}
