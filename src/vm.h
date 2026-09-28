#ifndef VM_H
#define VM_H

#include "tac.h"

// Backend: Stack Machine Code Emitter and Execution Engine
typedef enum {
    OP_HALT = 0,
    OP_PUSH_CONST,
    OP_LOAD_VAR,
    OP_STORE_VAR,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_LTE,
    OP_GT,
    OP_GTE,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_CALL,
    OP_RET,
    OP_PRINT,
    OP_ALLOC_STRUCT,
    OP_GET_FIELD,
    OP_SET_FIELD
} OpCode;

typedef struct {
    OpCode op;
    char arg1[64];
    char arg2[64];
    int int_arg;
} BytecodeInstr;

typedef struct {
    BytecodeInstr *instructions;
    int count;
    int capacity;
} BytecodeProgram;

// Compile TAC into Stack-Machine Bytecode
BytecodeProgram* vm_compile_tac(TACProgram *prog);

// Disassemble Stack-Machine Bytecode to stdout
void vm_disassemble(BytecodeProgram *code);

// Run the bytecode program (Backend runner/interpreter)
int vm_execute(BytecodeProgram *code);

void vm_free(BytecodeProgram *code);

#endif // VM_H
