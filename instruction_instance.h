#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////

typedef struct {
    uint8_t len : 8;
    uint32_t val : 24;
} opcode_t;

#define make_opcode(val_, len_) ((opcode_t) {.val = val_, .len = len_})

////////////////////////////////////////////////////////////////

typedef struct {
    uint8_t flag_w : 1;
    uint8_t flag_r : 1;
    uint8_t flag_x : 1;
    uint8_t flag_b : 1;
    uint8_t prefix_group_1 : 2;
    uint8_t prefix_group_2 : 3;
    uint8_t prefix_group_3 : 1;
    uint8_t prefix_group_4 : 1;
    uint8_t has_rex : 1;
} prefixes_legacy_t;

typedef struct {
    uint8_t flag_w : 1;
    uint8_t flag_r : 1;
    uint8_t flag_x : 1;
    uint8_t flag_b : 1;
    uint8_t size_256 : 1;
    uint8_t map_select : 5;
    uint8_t implicit : 2;
    uint8_t op_2 : 4;
    uint8_t vexsize_override : 1;
} prefixes_vex_t;

typedef struct {
    uint8_t instance_type : 2;
    uint8_t has_modrm : 1;
    uint8_t padding_1 : 1;
    uint8_t disp_size : 2;
    uint8_t imm_size : 2;
    union {
        prefixes_legacy_t legacy;
        prefixes_vex_t vex;
    };
} misc_t;

#define INSTR_TYPE_LEGACY 1
#define INSTR_TYPE_VEX    2
#define INSTR_TYPE_3DNOW  3

#define PREFIX_LOCK  1
#define PREFIX_REPNZ 2
#define PREFIX_REPZ  3

#define PREFIX_OVERRIDE_CS  1
#define PREFIX_OVERRIDE_SS  2
#define PREFIX_OVERRIDE_DS  3
#define PREFIX_OVERRIDE_ES  4
#define PREFIX_OVERRIDE_FS  5
#define PREFIX_OVERRIDE_GS  6

#define PREFIX_VEX_IMPLICIT_66 1
#define PREFIX_VEX_IMPLICIT_F3 2
#define PREFIX_VEX_IMPLICIT_F2 3

////////////////////////////////////////////////////////////////

typedef struct {
    uint8_t mod : 2;
    uint8_t reg : 3;
    uint8_t rm  : 3;
} modrm_t;

#define MOD_0 0b00
#define MOD_1 0b01
#define MOD_2 0b10
#define MOD_3 0b11
#define MOD_DIRECT MOD_3

#define make_modrm(mod_, reg_, rm_) ((modrm_t) { \
    .mod = mod_, \
    .reg = reg_, \
    .rm = rm_ \
})

typedef struct {
    uint8_t scale : 2;
    uint8_t index : 3;
    uint8_t base  : 3;
} sib_t;

#define make_sib(scale_, index_, base_) ((sib_t) { \
    .scale = scale_, \
    .index = index_, \
    .base = base_ \
})

////////////////////////////////////////////////////////////////

typedef struct {
    uint64_t disp;
    uint64_t imm;
    opcode_t opcode;
    misc_t misc;
    modrm_t modrm;
    sib_t sib;
} instr_instance_t;

#define instr_instantiation_error ((instr_instance_t) { \
    .disp = __LINE__, \
    .imm = (uint64_t) __FILE__ \
})

#define disp_size(instr) ((instr).misc.disp_size)
#define imm_size(instr) ((instr).misc.imm_size)
#define has_disp(instr) (disp_size(instr) != ARG_SIZE_NONE)
#define has_imm(instr) (imm_size(instr) != ARG_SIZE_NONE)
#define has_modrm(instr) ((instr).misc.has_modrm)
#define has_sib(instr) ( \
    ((instr).modrm.mod != 0b11) && \
    ((instr).modrm.rm == 0b100) \
)

#define opcode_len(instr) ((instr).opcode.len)
#define opcode_val(instr) ((instr).opcode.val)

#define instance_type(instr) ((instr).misc.instance_type)
#define instance_is_none(instr) (!instance_type(instr))
#define instance_is_legacy(instr) (instance_type(instr) == INSTR_TYPE_LEGACY)
#define instance_is_vex(instr) (instance_type(instr) == INSTR_TYPE_VEX)
#define instance_is_3dnow(instr) (instance_type(instr) == INSTR_TYPE_3DNOW)

#define prefix_legacy(instr) ((instr).misc.legacy)
#define prefix_vex(instr) ((instr).misc.vex)

#define prefix_group_1(instr) (prefix_legacy(instr).prefix_group_1)
#define prefix_group_2(instr) (prefix_legacy(instr).prefix_group_2)
#define prefix_group_3(instr) (prefix_legacy(instr).prefix_group_3)
#define prefix_group_4(instr) (prefix_legacy(instr).prefix_group_4)
#define opsize_override(instr) prefix_group_3(instr)
#define addrsize_override(instr) prefix_group_4(instr)

#define prefix_has_rex(instr) (prefix_legacy(instr).has_rex)
#define prefix_flag_b(instr) (prefix_legacy(instr).flag_b)
#define prefix_flag_x(instr) (prefix_legacy(instr).flag_x)
#define prefix_flag_r(instr) (prefix_legacy(instr).flag_r)
#define prefix_flag_w(instr) (prefix_legacy(instr).flag_w)

#define prefix_vex_map_select(instr) (prefix_vex(instr).map_select)
#define prefix_vex_op_2(instr) (prefix_vex(instr).op_2)
#define prefix_vex_256(instr) (prefix_vex(instr).size_256)
#define prefix_vex_implicit(instr) (prefix_vex(instr).implicit)
#define prefix_vex_vexsize_override(instr) (prefix_vex(instr).vexsize_override)

#define instance_is_invalid(instr) (opcode_len(instr) == 0)
#define instance_is_valid(instr) (opcode_len(instr) != 0)
