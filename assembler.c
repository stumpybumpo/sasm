#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "macro_helpers.h"

#include "buffer.h"
#include "argument.h"
#include "register_constants.h"
#include "instruction.h"
#include "instruction_instance.h"
#include "instruction_write.c"

////////////////////////////////////////////////////////////////

#define make_instr(op_, ...) \
((instr_t) { \
    .op = op_, \
    .args = (arg_t[num_args(__VA_ARGS__)]) {__VA_ARGS__}, \
    .len = num_args(__VA_ARGS__) \
})

#define NOP(...) make_instr(OP_NOP, __VA_ARGS__)
#define ADD(...) make_instr(OP_ADD, __VA_ARGS__)

////////////////////////////////////////////////////////////////

inline static void emit(buffer_t* buf, instr_t instr) {
    instr_instance_t instance = instruction_instantiate(instr);
    if (instance_is_valid(instance)) {
        printf("good instr: ");
        print_instr(instr);
        write_instruction_instance(buf, instance);
    }
    else {
        printf(
            "bad instruction returned at line %lu in %s: ",
            instance.disp,
            (char*) instance.imm
        );
        print_instr(instr);
    }
    buf->cursor = (((buf->cursor) >> 8) + 1) << 8;
}

////////////////////////////////////////////////////////////////

int main(void) {
    uint64_t bufsize = 4096;
    buffer_t buf = alloc_buf(bufsize);

    emit(&buf, NOP(arg_imm_8(1)));
    emit(&buf, NOP(arg_imm_8(2)));
    emit(&buf, NOP(arg_imm_8(3)));

    buf_write_64(&buf, 0);
    buf_write_64(&buf, 0);

    emit(&buf, ADD(AL, arg_imm_8(0xff)));
    emit(&buf, ADD(AX, arg_imm_16(0xffff)));
    emit(&buf, ADD(EAX, arg_imm_32(0xffffffff)));
    emit(&buf, ADD(RAX, arg_imm_32(0xffffffff)));
    emit(&buf, ADD(DL, arg_imm_8(0xff)));
    emit(&buf, ADD(R8L, arg_imm_8(0xff)));
    emit(&buf, ADD(SPL, arg_imm_8(0xff)));
    emit(&buf, ADD(arg_mem_64_base(RAX), arg_imm_32(0xffffffff)));
    emit(&buf, ADD(arg_mem_64(RIP, arg_reg_none, 0, 0, ARG_SIZE_32), arg_imm_32(0xffffffff)));
    emit(&buf, ADD(arg_mem_32(EIP, arg_reg_none, 0, 0, ARG_SIZE_32), arg_imm_32(0xffffffff)));
    emit(&buf, ADD(arg_mem_64(RAX, arg_reg_none, 0, 0, ARG_SIZE_32), RDX));

    buf_hexdump(buf);
    return 0;
}
