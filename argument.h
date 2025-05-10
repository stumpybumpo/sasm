#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////

#define ARG_TYPE_NONE   0
#define ARG_TYPE_REG    1
#define ARG_TYPE_MEM    2
#define ARG_TYPE_IMM    3
#define ARG_TYPE_MEMREG 4
#define ARG_TYPE_ANY    15

#define ARG_SIZE_NONE   0
#define ARG_SIZE_8      1
#define ARG_SIZE_16     2
#define ARG_SIZE_32     3
#define ARG_SIZE_64     4
#define ARG_SIZE_128    5
#define ARG_SIZE_256    6
#define ARG_SIZE_80     7
#define ARG_SIZE_ANY   15

////////////////////////////////////////////////////////////////

#include "register.h"
#include "memory.h"
#include "immediate.h"

typedef struct {
    union {
       register_t reg;
       memory_t mem;
       immediate_t imm;
    };
    uint8_t tag;
} arg_t;

#define arg_type(arg) ((arg).tag)
#define arg_is_none(arg) ((arg).tag == ARG_TYPE_NONE)
#define arg_is_reg(arg) ((arg).tag == ARG_TYPE_REG)
#define arg_is_mem(arg) ((arg).tag == ARG_TYPE_MEM)
#define arg_is_imm(arg) ((arg).tag == ARG_TYPE_IMM)

#define arg_to_reg(arg) ((arg).reg)
#define arg_to_mem(arg) ((arg).mem)
#define arg_to_imm(arg) ((arg).imm)

inline static uint8_t arg_size(arg_t arg) {
    switch (arg_type(arg)) {
    case ARG_TYPE_REG: return register_size(arg_to_reg(arg));
    case ARG_TYPE_MEM: return memory_size(arg_to_mem(arg));
    case ARG_TYPE_IMM: return immediate_size(arg_to_imm(arg));
    }
    return ARG_SIZE_NONE;
}

////////////////////////////////////////////////////////////////

arg_t arg_none = {.tag = ARG_TYPE_NONE};

inline static arg_t arg_reg(uint8_t type, uint8_t id) {
    return (arg_t) {
        .tag = ARG_TYPE_REG,
        .reg = {.type = type, .id = id}
    };
}

inline static arg_t arg_mem(arg_t    arg_base,
                            arg_t    arg_index,
                            uint8_t  scale,
                            uint64_t disp,
                            uint8_t  disp_size,
                            uint8_t  size) {
    if (!arg_is_reg(arg_index) || !arg_is_reg(arg_base)) {
        return arg_none;
    }

    register_t index = arg_to_reg(arg_index);
    register_t base = arg_to_reg(arg_base);

    /*if (!register_is_general(index) && !register_is_ip(index)) {
        return arg_none;
    }
    if (!register_is_general(base) && !register_is_ip(base)) {
        return arg_none;
    }
    if (register_size(index) != register_size(base)) {
        return arg_none;
    }
    if (register_is_ip(index)) {
        return arg_none;
    }
    if (register_is_8_no_rex(base) && register_is_8_rex(index)) {
        return arg_none;
    }
    if (register_is_8_no_rex(index) && register_is_8_rex(base)) {
        return arg_none;
    }
    if (register_size(base) > disp_size) {
        return arg_none;
    }
    if (disp_size != ARG_SIZE_NONE &&
        disp_size != ARG_SIZE_8 &&
        disp_size != ARG_SIZE_32) {
        return arg_none;
    }
    if ((scale == ARG_SIZE_NONE) && !register_is_none(index)) {
        return arg_none;
    }*/

    return (arg_t) {
        .tag = ARG_TYPE_MEM,
        .mem = {
            .scale = scale,
            .index = index,
            .base = base,
            .disp = disp,
            .disp_size = disp_size,
            .size = size
        }
    };
}

inline static arg_t arg_imm(uint64_t data, uint8_t size) {
    switch (size) {
    case ARG_SIZE_8:
        data &= 0xff;
        break;
    case ARG_SIZE_16:
        data &= 0xffff;
        break;
    case ARG_SIZE_32:
        data &= 0xffffffff;
        break;
    case ARG_SIZE_64:
        break;
    default:
        return arg_none;
    }
    return (arg_t) {
        .tag = ARG_TYPE_IMM,
        .imm = {.data = data, .size = size}
    };
}

////////////////////////////////////////////////////////////////

#define arg_reg_none arg_reg(REGISTER_TYPE_NONE, 0)
#define arg_reg_8(reg_id) arg_reg(REGISTER_TYPE_8_BIT, reg_id)
#define arg_reg_16(reg_id) arg_reg(REGISTER_TYPE_16_BIT, reg_id)
#define arg_reg_32(reg_id) arg_reg(REGISTER_TYPE_32_BIT, reg_id)
#define arg_reg_64(reg_id) arg_reg(REGISTER_TYPE_64_BIT, reg_id)
#define arg_reg_8_rex(reg_id) arg_reg(REGISTER_TYPE_8_BIT_REX, reg_id)
#define arg_reg_32_ip(reg_id) arg_reg(REGISTER_TYPE_32_BIT_IP, reg_id)
#define arg_reg_64_ip(reg_id) arg_reg(REGISTER_TYPE_64_BIT_IP, reg_id)
#define arg_reg_x87(reg_id) arg_reg(REGISTER_TYPE_X87, reg_id)
#define arg_reg_mmx(reg_id) arg_reg(REGISTER_TYPE_MMX, reg_id)
#define arg_reg_xmm(reg_id) arg_reg(REGISTER_TYPE_XMM, reg_id)
#define arg_reg_ymm(reg_id) arg_reg(REGISTER_TYPE_YMM, reg_id)
#define arg_reg_segment(reg_id) arg_reg(REGISTER_TYPE_SEGMENT, reg_id)
#define arg_reg_control(reg_id) arg_reg(REGISTER_TYPE_CONTROL, reg_id)
#define arg_reg_debug(reg_id) arg_reg(REGISTER_TYPE_DEBUG, reg_id)

#define arg_mem_8(base, index, scale, disp, disp_size) \
    arg_mem(base, index, scale, disp, disp_size, ARG_SIZE_8)
#define arg_mem_16(base, index, scale, disp, disp_size) \
    arg_mem(base, index, scale, disp, disp_size, ARG_SIZE_16)
#define arg_mem_32(base, index, scale, disp, disp_size) \
    arg_mem(base, index, scale, disp, disp_size, ARG_SIZE_32)
#define arg_mem_64(base, index, scale, disp, disp_size) \
    arg_mem(base, index, scale, disp, disp_size, ARG_SIZE_64)

#define arg_mem_8_disp_8(disp) \
    arg_mem_8(arg_reg_none, arg_reg_none, 0, disp, ARG_SIZE_8)
#define arg_mem_16_disp_8(disp) \
    arg_mem_16(arg_reg_none, arg_reg_none, 0, disp, ARG_SIZE_8)
#define arg_mem_16_disp_16(disp) \
    arg_mem_16(arg_reg_none, arg_reg_none, 0, disp, ARG_SIZE_16)
#define arg_mem_32_disp_8(disp) \
    arg_mem_32(arg_reg_none, arg_reg_none, 0, disp, ARG_SIZE_8)
#define arg_mem_32_disp_16(disp) \
    arg_mem_32(arg_reg_none, arg_reg_none, 0, disp, ARG_SIZE_16)
#define arg_mem_32_disp_32(disp) \
    arg_mem_32(arg_reg_none, arg_reg_none, 0, disp, ARG_SIZE_32)
#define arg_mem_64_disp_8(disp) \
    arg_mem_64(arg_reg_none, arg_reg_none, 0, disp, ARG_SIZE_8)
#define arg_mem_64_disp_16(disp) \
    arg_mem_64(arg_reg_none, arg_reg_none, 0, disp, ARG_SIZE_16)
#define arg_mem_64_disp_32(disp) \
    arg_mem_64(arg_reg_none, arg_reg_none, 0, disp, ARG_SIZE_32)
#define arg_mem_64_disp_64(disp) \
    arg_mem_64(arg_reg_none, arg_reg_none, 0, disp, ARG_SIZE_64)

#define arg_mem_8_base(base) \
    arg_mem_8(base, arg_reg_none, 0, 0, ARG_SIZE_NONE)
#define arg_mem_16_base(base) \
    arg_mem_16(base, arg_reg_none, 0, 0, ARG_SIZE_NONE)
#define arg_mem_32_base(base) \
    arg_mem_32(base, arg_reg_none, 0, 0, ARG_SIZE_NONE)
#define arg_mem_64_base(base) \
    arg_mem_64(base, arg_reg_none, 0, 0, ARG_SIZE_NONE)

#define arg_imm_8(data) arg_imm(data, ARG_SIZE_8)
#define arg_imm_16(data) arg_imm(data, ARG_SIZE_16)
#define arg_imm_32(data) arg_imm(data, ARG_SIZE_32)
#define arg_imm_64(data) arg_imm(data, ARG_SIZE_64)

////////////////////////////////////////////////////////////////

inline static void print_arg(arg_t arg) {
    switch (arg_type(arg)) {
    case ARG_TYPE_REG:
        print_reg(arg_to_reg(arg));
        break;
    case ARG_TYPE_IMM:
        print_imm(arg_to_imm(arg));
        break;
    case ARG_TYPE_MEM:
        print_mem(arg_to_mem(arg));
        break;
    default:
        printf("ARG_BAD");
        break;
    }
}
