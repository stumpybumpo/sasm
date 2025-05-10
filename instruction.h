#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////

typedef enum {OP_ADD, OP_NOP} op_t;

typedef struct {
    arg_t* args;
    op_t op;
    uint8_t len;
} instr_t;

////////////////////////////////////////////////////////////////

inline static void print_instr(instr_t instr) {
    switch (instr.op) {
    case OP_NOP:
        printf("nop(");
        break;
    case OP_ADD:
        printf("add(");
        break;
    }
    for (int i = 0; i < instr.len; i++) {
        if (i != 0) {
            printf(", ");
        }
        print_arg(instr.args[i]);
    }
    printf(")\n");
}
