#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////

typedef struct {
    uint8_t id;
    uint8_t size : 4;
    uint8_t type : 4;
} arg_info_t;

#define reg_type_id(size_, id_) ((arg_info_t) {.type = ARG_TYPE_REG, .size = size_, .id = id_})
#define reg_type(size_) (reg_type_id(size_, -1))
#define mem_type(size_) ((arg_info_t) {.type = ARG_TYPE_MEM, .size = size_, .id = -1})
#define memreg_type(size_) ((arg_info_t) {.type = ARG_TYPE_MEMREG, .size = size_, .id = -1})
#define imm_type(size_) ((arg_info_t) {.type = ARG_TYPE_IMM, .size = size_, .id = -1})

#define arg_info_type(arg_info) ((arg_info).type)
#define arg_info_size(arg_info) ((arg_info).size)
#define arg_info_id(arg_info) ((arg_info).id)

typedef struct {
    arg_info_t* args_info;
    opcode_t opcode;
    uint8_t len;
} instr_schema_t;

typedef struct {
    instr_schema_t* schemata;
    uint32_t len;
} instr_schemata_t;

static inline uint32_t match_schema(instr_schemata_t schemata, instr_t instr) {
    for (int i = 0; i < schemata.len; i++) {
        instr_schema_t schema = schemata.schemata[i];
        uint8_t arg_num = 0;
        if (schema.len != instr.len) continue;
        for (int j = 0; j < schema.len; j++) {
            arg_info_t arg_info = schema.args_info[j];
            arg_t arg = instr.args[arg_num];
            if (arg_info_type(arg_info) != ARG_TYPE_ANY) {
                if (arg_info_type(arg_info) == ARG_TYPE_MEMREG) {
                    if (arg_type(arg) != ARG_TYPE_MEM && arg_type(arg) != ARG_TYPE_REG) {
                        break;
                    }
                }
                else if (arg_info_type(arg_info) != arg_type(arg)) {
                    break;
                }
            }
            if (arg_info_type(arg_info) == ARG_TYPE_REG) {
                uint8_t reg_id = arg_info_id(arg_info);
                if ((reg_id != (uint8_t) -1) &&
                    (reg_id != register_id(arg_to_reg(arg)))) {
                    break;
                }
            }
            if (arg_info_size(arg_info) != ARG_SIZE_ANY) {
                if (arg_info_size(arg_info) != arg_size(arg)) {
                    break;
                }
            }

            arg_num++;
        }
        if (arg_num == schema.len) return i;
    }
    return -1;
}

////////////////////////////////////////////////////////////////

#define make_instr_schema(opcode_, ...) \
{ \
    .opcode = opcode_, \
    .args_info = ((arg_info_t[num_args(__VA_ARGS__)]) {__VA_ARGS__}), \
    .len = num_args(__VA_ARGS__) \
}

#define make_instr_schemata(...) \
{ \
    .schemata = ((instr_schema_t[num_args(__VA_ARGS__)]) {__VA_ARGS__}), \
    .len = num_args(__VA_ARGS__) \
}

////////////////////////////////////////////////////////////////

instr_schemata_t nop_schemata = make_instr_schemata(
    make_instr_schema(make_opcode(0x90, 1), imm_type(ARG_SIZE_8)),
    make_instr_schema(make_opcode(0x90, 1), imm_type(ARG_SIZE_8)),
    make_instr_schema(make_opcode(0x0f1f, 2), imm_type(ARG_SIZE_8))
);

instr_schemata_t add_schemata = make_instr_schemata(
    make_instr_schema(make_opcode(0x04, 1), reg_type_id(ARG_SIZE_8, 0), imm_type(ARG_SIZE_8)),
    make_instr_schema(make_opcode(0x05, 1), reg_type_id(ARG_SIZE_16, 0), imm_type(ARG_SIZE_16)),
    make_instr_schema(make_opcode(0x05, 1), reg_type_id(ARG_SIZE_32, 0), imm_type(ARG_SIZE_32)),
    make_instr_schema(make_opcode(0x05, 1), reg_type_id(ARG_SIZE_64, 0), imm_type(ARG_SIZE_32)),
    make_instr_schema(make_opcode(0x80, 1), memreg_type(ARG_SIZE_8), imm_type(ARG_SIZE_8)),
    make_instr_schema(make_opcode(0x81, 1), memreg_type(ARG_SIZE_16), imm_type(ARG_SIZE_16)),
    make_instr_schema(make_opcode(0x81, 1), memreg_type(ARG_SIZE_32), imm_type(ARG_SIZE_32)),
    make_instr_schema(make_opcode(0x81, 1), memreg_type(ARG_SIZE_64), imm_type(ARG_SIZE_32)),
    make_instr_schema(make_opcode(0x83, 1), memreg_type(ARG_SIZE_16), imm_type(ARG_SIZE_8)),
    make_instr_schema(make_opcode(0x83, 1), memreg_type(ARG_SIZE_32), imm_type(ARG_SIZE_8)),
    make_instr_schema(make_opcode(0x83, 1), memreg_type(ARG_SIZE_64), imm_type(ARG_SIZE_8)),
    make_instr_schema(make_opcode(0x00, 1), memreg_type(ARG_SIZE_8), reg_type(ARG_SIZE_8)),
    make_instr_schema(make_opcode(0x01, 1), memreg_type(ARG_SIZE_16), reg_type(ARG_SIZE_16)),
    make_instr_schema(make_opcode(0x01, 1), memreg_type(ARG_SIZE_32), reg_type(ARG_SIZE_32)),
    make_instr_schema(make_opcode(0x01, 1), memreg_type(ARG_SIZE_64), reg_type(ARG_SIZE_64)),
    make_instr_schema(make_opcode(0x02, 1), reg_type(ARG_SIZE_8), memreg_type(ARG_SIZE_8)),
    make_instr_schema(make_opcode(0x03, 1), reg_type(ARG_SIZE_16), memreg_type(ARG_SIZE_16)),
    make_instr_schema(make_opcode(0x03, 1), reg_type(ARG_SIZE_32), memreg_type(ARG_SIZE_32)),
    make_instr_schema(make_opcode(0x03, 1), reg_type(ARG_SIZE_64), memreg_type(ARG_SIZE_64))
);
