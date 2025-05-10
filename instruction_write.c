#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "instruction_schemata.h"
#include "instruction_encoding_legacy.c"
#include "instruction_encoding_vex.c"

////////////////////////////////////////////////////////////////

static inline void write_opcode(buffer_t* buf, instr_instance_t instance);
static inline void write_disp(buffer_t* buf, instr_instance_t instance);
static inline void write_imm(buffer_t* buf, instr_instance_t instance);
static inline void write_modrm_sib(buffer_t* buf, instr_instance_t instance);

static inline void write_legacy_instance(buffer_t*        buf,
                                         instr_instance_t instance) {
    switch (prefix_group_1(instance)) {
    case PREFIX_LOCK:
        buf_write_8(buf, 0xf0);
        break;
    case PREFIX_REPNZ:
        buf_write_8(buf, 0xf2);
        break;
    case PREFIX_REPZ:
        buf_write_8(buf, 0xf3);
        break;
    }
    switch (prefix_group_2(instance)) {
    case PREFIX_OVERRIDE_CS:
        buf_write_8(buf, 0x2e);
        break;
    case PREFIX_OVERRIDE_SS:
        buf_write_8(buf, 0x36);
        break;
    case PREFIX_OVERRIDE_DS:
        buf_write_8(buf, 0x3e);
        break;
    case PREFIX_OVERRIDE_ES:
        buf_write_8(buf, 0x26);
        break;
    case PREFIX_OVERRIDE_FS:
        buf_write_8(buf, 0x64);
        break;
    case PREFIX_OVERRIDE_GS:
        buf_write_8(buf, 0x65);
        break;
    }
    if (prefix_group_3(instance)) {
        buf_write_8(buf, 0x66);
    }
    if (prefix_group_4(instance)) {
        buf_write_8(buf, 0x67);
    }
    if (prefix_has_rex(instance)) {
        uint8_t rex_byte = 0x40;
        rex_byte |= prefix_flag_w(instance) << 3;
        rex_byte |= prefix_flag_r(instance) << 2;
        rex_byte |= prefix_flag_x(instance) << 1;
        rex_byte |= prefix_flag_b(instance);
        buf_write_8(buf, rex_byte);
    }
    write_opcode(buf, instance);
    write_modrm_sib(buf, instance);
    write_disp(buf, instance);
    write_imm(buf, instance);
}

static inline void write_vex_instance(buffer_t*        buf,
                                      instr_instance_t instance) {
    bool prefix_vex_short = !prefix_flag_x(instance) &&
                            !prefix_flag_b(instance) &&
                            !prefix_flag_w(instance) &&
                            (prefix_vex_map_select(instance) == 1);
    if (prefix_vex_short || prefix_vex_vexsize_override(instance)) {
        uint8_t vex_byte_1 = 0;
        vex_byte_1 ^= prefix_flag_r(instance) << 7;
        vex_byte_1 ^= prefix_vex_op_2(instance) << 3;
        vex_byte_1 |= prefix_vex_256(instance) << 2;
        vex_byte_1 |= prefix_vex_implicit(instance);
        buf_write_8(buf, 0xc5);
        buf_write_8(buf, vex_byte_1);
    }
    else {
        uint8_t vex_byte_1 = 0;
        vex_byte_1 ^= prefix_flag_r(instance) << 7;
        vex_byte_1 ^= prefix_flag_x(instance) << 6;
        vex_byte_1 ^= prefix_flag_b(instance) << 5;
        vex_byte_1 |= prefix_vex_map_select(instance);
        uint8_t vex_byte_2 = 0;
        vex_byte_2 |= prefix_flag_w(instance) << 7;
        vex_byte_2 ^= prefix_vex_op_2(instance) << 3;
        vex_byte_2 |= prefix_vex_256(instance) << 2;
        vex_byte_2 |= prefix_vex_implicit(instance);
        buf_write_8(buf, 0xc4);
        buf_write_8(buf, vex_byte_1);
        buf_write_8(buf, vex_byte_2);
    }
    write_opcode(buf, instance);
    write_modrm_sib(buf, instance);
    write_disp(buf, instance);
    write_imm(buf, instance);

}

static inline void write_3dnow_instance(buffer_t*        buf,
                                        instr_instance_t instance) {
    buf_write_16(buf, 0x0f0f);
    write_modrm_sib(buf, instance);
    write_disp(buf, instance);
    write_opcode(buf, instance);
}

static inline void write_nop_instance(buffer_t*        buf,
                                      instr_instance_t instance) {
    for (int i = 1; i < instance_nop_length(instance); i++) {
        buf_write_8(buf, 0x66);
    }
    write_opcode(buf, instance);
}

static inline void write_instruction_instance(buffer_t*        buf,
                                              instr_instance_t instance) {
    if (instance_is_invalid(instance)) {
        return;
    }

    switch (instance_type(instance)) {
    case INSTR_TYPE_LEGACY:
        write_legacy_instance(buf, instance);
        break;
    case INSTR_TYPE_VEX:
        write_vex_instance(buf, instance);
        break;
    case INSTR_TYPE_3DNOW:
        write_3dnow_instance(buf, instance);
        break;
    case INSTR_TYPE_NOP:
        write_nop_instance(buf, instance);
        break;
    }
}

static inline void write_opcode(buffer_t* buf, instr_instance_t instance) {
    uint32_t opcode_val = opcode_val(instance);
    if (opcode_len(instance) == 1) {
        buf_write_8(buf, opcode_val & 0xff);
        return;
    }
    if (opcode_len(instance) == 2) {
        buf_write_8(buf, (opcode_val >> 8) & 0xff);
        buf_write_8(buf, opcode_val & 0xff);
        return;
    }
    if (opcode_len(instance) == 3) {
        buf_write_8(buf, (opcode_val >> 16) & 0xff);
        buf_write_8(buf, (opcode_val >> 8) & 0xff);
        buf_write_8(buf, opcode_val & 0xff);
        return;
    }
    printf("weird opcode length\n");
}

static inline void write_disp(buffer_t* buf, instr_instance_t instance) {
    switch (disp_size(instance)) {
    case ARG_SIZE_NONE:
        return;
    case ARG_SIZE_8:
        buf_write_8(buf, instance.disp & 0xff);
        return;
    case ARG_SIZE_16:
        buf_write_16(buf, instance.disp & 0xffff);
        return;
    case ARG_SIZE_32:
        buf_write_32(buf, instance.disp & 0xffffffff);
        return;
    case ARG_SIZE_64:
        buf_write_64(buf, instance.disp);
        return;
    }
}

static inline void write_imm(buffer_t* buf, instr_instance_t instance) {
    switch (imm_size(instance)) {
    case ARG_SIZE_NONE:
        return;
    case ARG_SIZE_8:
        buf_write_8(buf, instance.imm & 0xff);
        return;
    case ARG_SIZE_16:
        buf_write_16(buf, instance.imm & 0xffff);
        return;
    case ARG_SIZE_32:
        buf_write_32(buf, instance.imm & 0xffffffff);
        return;
    case ARG_SIZE_64:
        buf_write_64(buf, instance.imm);
        return;
    }
}

static inline void write_modrm_sib(buffer_t* buf, instr_instance_t instance) {
    if (has_modrm(instance)) {
        modrm_t modrm = instance.modrm;
        uint8_t modrm_byte = 0;
        modrm_byte |= modrm.mod << 6;
        modrm_byte |= modrm.reg << 3;
        modrm_byte |= modrm.rm;
        buf_write_8(buf, modrm_byte);

        if (has_sib(instance)) {
            sib_t sib = instance.sib;
            uint8_t sib_byte = 0;
            sib_byte |= sib.scale << 6;
            sib_byte |= sib.index << 3;
            sib_byte |= sib.base;
            buf_write_8(buf, sib_byte);
        }
    }
}
