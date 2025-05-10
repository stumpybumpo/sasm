#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "instruction_schemata.h"

////////////////////////////////////////////////////////////////

static inline instr_instance_t add_sizes_legacy(instr_instance_t instance,
                                                instr_t          instr);

static inline instr_instance_t add_args_legacy(instr_instance_t instance,
                                               arg_t            dest,
                                               arg_t            src);

static inline instr_instance_t add_args_memory_legacy(instr_instance_t instance,
                                                      memory_t         mem,
                                                      arg_t            arg);

////////////////////////////////////////////////////////////////

static inline instr_instance_t instantiate_legacy(instr_t          instr,
                                                  instr_schemata_t schemata) {

    uint32_t match_id = match_schema(schemata, instr);
    if (match_id == -1) {
        return instr_instantiation_error;
    }

    instr_schema_t match = schemata.schemata[match_id];

    instr_instance_t instance = {0};

    instance.opcode = match.opcode;
    prefix_type(instance) = PREFIX_TYPE_LEGACY;

    instance = add_sizes_legacy(instance, instr);
    if (instance_is_invalid(instance)) {
        return instr_instantiation_error;
    }

    arg_t dest = arg_none;
    arg_t src = arg_none;

    switch (instr.len) {
    case 3:
        if (!arg_is_imm(instr.args[2])) {
            return instr_instantiation_error;
        }
        immediate_t imm = arg_to_imm(src);
        imm_size(instance) = immediate_size(imm);
        instance.imm = immediate_data(imm);
    case 2:
        if (arg_info_id(match.args_info[1]) == (uint8_t) -1) {
            src = instr.args[1];
        }
    case 1:
        if (arg_info_id(match.args_info[0]) == (uint8_t) -1) {
            dest = instr.args[0];
        }
    case 0:
        break;
    default:
        return instr_instantiation_error;
    }

    return add_args_legacy(instance, dest, src);
}

////////////////////////////////////////////////////////////////

static inline instr_instance_t add_sizes_legacy(instr_instance_t instance,
                                                instr_t          instr) {
    uint8_t op_size = ARG_SIZE_NONE;
    uint8_t addr_size = ARG_SIZE_NONE;

    for (int i = 0; i < instr.len; i++) {
        arg_t arg = instr.args[i];
        if (arg_is_mem(arg)) {
            if (addr_size == ARG_SIZE_NONE) {
                addr_size = arg_size(arg);
            }
            else {
                return instr_instantiation_error;
            }
        }
        if (arg_is_reg(arg)) {
            if (op_size == ARG_SIZE_NONE) {
                op_size = arg_size(arg);
            }
            else {
                return instr_instantiation_error;
            }
        }
    }

    switch (addr_size) {
    case ARG_SIZE_NONE:
        break;
    case ARG_SIZE_8:
    case ARG_SIZE_16:
        return instr_instantiation_error;
    case ARG_SIZE_32:
        addrsize_override(instance) = true;
        break;
    case ARG_SIZE_64:
        break;
    }

    switch (op_size) {
    case ARG_SIZE_NONE:
    case ARG_SIZE_8:
        break;
    case ARG_SIZE_16:
        opsize_override(instance) = true;
    case ARG_SIZE_32:
        break;
    case ARG_SIZE_64:
        prefix_flag_w(instance) = true;
        prefix_has_rex(instance) = true;
        break;
    }

    return instance;
}

////////////////////////////////////////////////////////////////

static inline instr_instance_t add_args_legacy(instr_instance_t instance,
                                               arg_t            dest,
                                               arg_t            src) {
    if (arg_is_mem(dest)) {
        return add_args_memory_legacy(instance, arg_to_mem(dest), src);
    }
    if (arg_is_mem(src)) {
        return add_args_memory_legacy(instance, arg_to_mem(src), dest);
    }

    if (arg_is_imm(src)) {
        immediate_t imm = arg_to_imm(src);
        imm_size(instance) = immediate_size(imm);
        instance.imm = immediate_data(imm);
        if (arg_is_none(dest)) {
            return instance;
        }
    }

    if (arg_is_none(dest)) {
        dest = src;
    }

    if (!arg_is_reg(dest)) {
        return instr_instantiation_error;
    }

    has_modrm(instance) = true;
    register_t reg_dest = arg_to_reg(dest);
    uint8_t reg_dest_id = register_id_low(reg_dest);
    if (register_id_high(reg_dest)) {
        prefix_flag_r(instance) = true;
        prefix_has_rex(instance) = true;
    }
    if (register_is_8_rex(reg_dest)) {
        prefix_has_rex(instance) = true;
    }

    if (arg_is_imm(src)) {
        instance.modrm = make_modrm(MOD_DIRECT, 0, reg_dest_id);
        return instance;
    }
    else if (!arg_is_reg(src)) {
        return instr_instantiation_error;
    }

    register_t reg_src = arg_to_reg(src);
    uint8_t reg_src_id = register_id_low(reg_src);
    if (register_id_high(reg_src)) {
        prefix_flag_b(instance) = true;
        prefix_has_rex(instance) = true;
    }
    if (register_is_8_rex(reg_src)) {
        prefix_has_rex(instance) = true;
    }

    instance.modrm = make_modrm(MOD_DIRECT, reg_src_id, reg_dest_id);
    return instance;
}

////////////////////////////////////////////////////////////////

static inline instr_instance_t add_args_memory_legacy(instr_instance_t instance,
                                                      memory_t         mem,
                                                      arg_t            arg) {
    if (arg_is_mem(arg)) {
        return instr_instantiation_error;
    }

    has_modrm(instance) = true;

    uint8_t reg_id = 0;

    if (arg_is_reg(arg)) {
        register_t reg = arg_to_reg(arg);
        reg_id = register_id_low(reg);
        if (register_id_high(reg)) {
            prefix_flag_r(instance) = true;
            prefix_has_rex(instance) = true;
        }
        if (register_is_8_rex(reg)) {
            prefix_has_rex(instance) = true;
        }
    }
    else if (arg_is_imm(arg)) {
        immediate_t imm = arg_to_imm(arg);
        imm_size(instance) = immediate_size(imm);
        instance.imm = immediate_data(imm);
    }
    else if (!arg_is_none(arg)) {
        return instr_instantiation_error;
    }

    // arg is fully handled and we never need to look at it again beyond reg_id

    register_t base = memory_base(mem);
    uint8_t base_id = register_id_low(base);
    register_t index = memory_index(mem);
    uint8_t index_id = register_id_low(index);
    uint8_t scale = memory_scale(mem);
    uint64_t disp = memory_disp(mem);

    uint8_t disp_size = memory_disp_size(mem);

    switch (scale) {
    case MEMORY_SCALE_1:
        scale = 0b00;
        break;
    case MEMORY_SCALE_2:
        scale = 0b01;
        break;
    case MEMORY_SCALE_4:
        scale = 0b10;
        break;
    case MEMORY_SCALE_8:
        scale = 0b11;
        break;
    }

    if (disp_size == ARG_SIZE_NONE ||
        disp_size == ARG_SIZE_8 ||
        disp_size == ARG_SIZE_32) {
        disp_size(instance) = disp_size;
        instance.disp = disp;
    }
    else {
        return instr_instantiation_error;
    }

    uint8_t sib_id = 0b100; // sp
    uint8_t ip_id = 0b101;  // bp

    if (register_is_ip(base)) {
        if (!(disp_size == ARG_SIZE_32) || !register_is_none(index)) {
            return instr_instantiation_error;
        }
        instance.modrm = make_modrm(MOD_0, reg_id, ip_id);
        return instance;
    }

    if (register_is_none(base)) {
        if (disp_size != ARG_SIZE_32) {
            return instr_instantiation_error;
        }
        instance.modrm = make_modrm(MOD_0, reg_id, sib_id);
        if (register_is_none(index)) {
            instance.sib = make_sib(0, sib_id, ip_id);
        }
        else {
            if (index_id == sib_id) {
                return instr_instantiation_error;
            }
            instance.sib = make_sib(scale, sib_id, ip_id);
        }
        return instance;
    }

    if (register_is_none(index)) {
        if ((base_id == ip_id) && !memory_has_disp(mem)) {
            return instr_instantiation_error;
        }

        instance.modrm = make_modrm(disp_size, reg_id, base_id);
        if (base_id == sib_id) {
            instance.sib = make_sib(0, sib_id, base_id);
        }
        return instance;
    }

    instance.modrm = make_modrm(disp_size, reg_id, sib_id);

    if (register_id_high(index)) {
        prefix_flag_x(instance) = true;
        prefix_has_rex(instance) = true;
    }
    if (register_id_high(base)) {
        prefix_flag_b(instance) = true;
        prefix_has_rex(instance) = true;
    }

    instance.sib = make_sib(scale, index_id, base_id);
    return instance;
}

////////////////////////////////////////////////////////////////

static inline instr_instance_t instantiate_nop(instr_t instr);

static inline instr_instance_t instruction_instantiate(instr_t instr) {
    switch (instr.op) {
    case OP_NOP:
        return instantiate_nop(instr);
    case OP_ADD:
        return instantiate_legacy(instr, add_schemata);
    default:
        return instr_instantiation_error;
    }
}

static inline instr_instance_t instantiate_nop(instr_t instr) {
    if (!arg_is_imm(instr.args[0]) || arg_size(instr.args[0]) != ARG_SIZE_8) {
        return instr_instantiation_error;
    }

    uint8_t nop_length = immediate_data(arg_to_imm(instr.args[0]));
    instr_instance_t instance = {0};
    prefix_type(instance) = PREFIX_TYPE_LEGACY;

    switch (nop_length) {
    case 1:
        instance.opcode = make_opcode(0x90, 1);
        return instance;
    case 2:
        instance.opcode = make_opcode(0x90, 1);
        opsize_override(instance) = true;
        return instance;
    case 3:
        instance.opcode = make_opcode(0x0f1f, 2);
        has_modrm(instance) = true;
        instance.modrm = make_modrm(0, 0, 0);
        return instance;
    // TODO add more (longer) nops
    }
    return instr_instantiation_error;
}
