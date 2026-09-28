#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vm.h"

// Dynamic struct instance representation in VM heap
typedef struct StructFieldVal {
    char name[64];
    int int_val;
    char str_val[128];
    struct StructFieldVal *next;
} StructFieldVal;

typedef struct StructObj {
    char struct_name[64];
    StructFieldVal *fields;
} StructObj;

// VM Value
typedef enum {
    VAL_INT,
    VAL_STR,
    VAL_OBJ
} ValType;

typedef struct {
    ValType type;
    int int_val;
    char str_val[128];
    StructObj *obj_val;
} VMVal;

// Local variable environment (Frame)
typedef struct VarEntry {
    char name[64];
    VMVal val;
    struct VarEntry *next;
} VarEntry;

typedef struct CallFrame {
    char func_name[64];
    int return_ip;
    VarEntry *locals;
    struct CallFrame *parent_frame; // Static/lexical link for nested functions
} CallFrame;

// Bytecode Program builder
static void emit(BytecodeProgram *code, OpCode op, const char *a1, const char *a2, int int_arg) {
    if (code->count >= code->capacity) {
        code->capacity = code->capacity == 0 ? 64 : code->capacity * 2;
        code->instructions = (BytecodeInstr*)realloc(code->instructions, sizeof(BytecodeInstr) * code->capacity);
    }
    BytecodeInstr *i = &code->instructions[code->count++];
    i->op = op;
    i->int_arg = int_arg;
    if (a1) strncpy(i->arg1, a1, 63); else i->arg1[0] = '\0';
    if (a2) strncpy(i->arg2, a2, 63); else i->arg2[0] = '\0';
}

static void emit_push_operand(BytecodeProgram *code, const char *arg) {
    if (!arg || strlen(arg) == 0) return;

    if (isdigit(arg[0]) || (arg[0] == '-' && isdigit(arg[1]))) {
        emit(code, OP_PUSH_CONST, arg, NULL, atoi(arg));
    } else if (arg[0] == '"') {
        // String literal without quotes
        char buf[128];
        int len = strlen(arg);
        if (len >= 2) {
            strncpy(buf, arg + 1, len - 2);
            buf[len - 2] = '\0';
        } else {
            strcpy(buf, "");
        }
        emit(code, OP_PUSH_CONST, buf, "str", 0);
    } else {
        emit(code, OP_LOAD_VAR, arg, NULL, 0);
    }
}

BytecodeProgram* vm_compile_tac(TACProgram *prog) {
    BytecodeProgram *code = (BytecodeProgram*)calloc(1, sizeof(BytecodeProgram));
    if (!prog) return code;

    for (TACInstr *i = prog->head; i; i = i->next) {
        switch (i->op) {
            case TAC_FUNC_BEGIN:
                emit(code, OP_HALT, i->result, "FUNC_LABEL", 0);
                break;
            case TAC_FUNC_END:
                emit(code, OP_RET, NULL, NULL, 0);
                break;
            case TAC_LABEL:
                emit(code, OP_HALT, i->result, "LABEL", 0);
                break;

            case TAC_ASSIGN:
                emit_push_operand(code, i->arg1);
                emit(code, OP_STORE_VAR, i->result, NULL, 0);
                break;

            case TAC_ADD:
            case TAC_SUB:
            case TAC_MUL:
            case TAC_DIV:
            case TAC_EQ:
            case TAC_NEQ:
            case TAC_LT:
            case TAC_LTE:
            case TAC_GT:
            case TAC_GTE:
            case TAC_AND:
            case TAC_OR: {
                emit_push_operand(code, i->arg1);
                emit_push_operand(code, i->arg2);
                OpCode op = OP_ADD;
                if (i->op == TAC_SUB) op = OP_SUB;
                else if (i->op == TAC_MUL) op = OP_MUL;
                else if (i->op == TAC_DIV) op = OP_DIV;
                else if (i->op == TAC_EQ)  op = OP_EQ;
                else if (i->op == TAC_NEQ) op = OP_NEQ;
                else if (i->op == TAC_LT)  op = OP_LT;
                else if (i->op == TAC_LTE) op = OP_LTE;
                else if (i->op == TAC_GT)  op = OP_GT;
                else if (i->op == TAC_GTE) op = OP_GTE;
                else if (i->op == TAC_AND) op = OP_AND;
                else if (i->op == TAC_OR)  op = OP_OR;
                emit(code, op, NULL, NULL, 0);
                emit(code, OP_STORE_VAR, i->result, NULL, 0);
                break;
            }

            case TAC_NOT:
                emit_push_operand(code, i->arg1);
                emit(code, OP_NOT, NULL, NULL, 0);
                emit(code, OP_STORE_VAR, i->result, NULL, 0);
                break;

            case TAC_NEG:
                emit(code, OP_PUSH_CONST, "0", NULL, 0);
                emit_push_operand(code, i->arg1);
                emit(code, OP_SUB, NULL, NULL, 0);
                emit(code, OP_STORE_VAR, i->result, NULL, 0);
                break;

            case TAC_IF_FALSE:
                emit_push_operand(code, i->arg1);
                emit(code, OP_JUMP_IF_FALSE, i->arg2, NULL, 0);
                break;

            case TAC_GOTO:
                emit(code, OP_JUMP, i->arg1, NULL, 0);
                break;

            case TAC_PARAM:
                emit_push_operand(code, i->arg1);
                break;

            case TAC_PARAM_POP:
                emit(code, OP_STORE_VAR, i->result, NULL, 0);
                break;

            case TAC_CALL:
                emit(code, OP_CALL, i->arg1, NULL, atoi(i->arg2));
                emit(code, OP_STORE_VAR, i->result, NULL, 0);
                break;

            case TAC_RETURN:
                if (strlen(i->arg1) > 0) {
                    emit_push_operand(code, i->arg1);
                }
                emit(code, OP_RET, NULL, NULL, 0);
                break;

            case TAC_PRINT:
                emit_push_operand(code, i->arg1);
                emit(code, OP_PRINT, NULL, NULL, 0);
                break;

            case TAC_NEW:
                emit(code, OP_ALLOC_STRUCT, i->arg1, NULL, 0);
                emit(code, OP_STORE_VAR, i->result, NULL, 0);
                break;

            case TAC_GET_FIELD:
                emit_push_operand(code, i->arg1);
                emit(code, OP_GET_FIELD, i->arg2, NULL, 0);
                emit(code, OP_STORE_VAR, i->result, NULL, 0);
                break;

            case TAC_SET_FIELD:
                emit_push_operand(code, i->result);
                emit_push_operand(code, i->arg1);
                emit(code, OP_SET_FIELD, i->arg2, NULL, 0);
                break;

            default:
                break;
        }
    }

    emit(code, OP_HALT, NULL, NULL, 0);
    return code;
}

void vm_disassemble(BytecodeProgram *code) {
    if (!code) return;
    printf("\n=== STACK-MACHINE BYTECODE DISASSEMBLY ===\n");
    for (int ip = 0; ip < code->count; ip++) {
        BytecodeInstr *i = &code->instructions[ip];
        printf("[%04d] ", ip);
        switch (i->op) {
            case OP_HALT:
                if (strcmp(i->arg2, "LABEL") == 0 || strcmp(i->arg2, "FUNC_LABEL") == 0)
                    printf("%s:\n", i->arg1);
                else
                    printf("HALT\n");
                break;
            case OP_PUSH_CONST:
                if (strcmp(i->arg2, "str") == 0) printf("PUSH_STR \"%s\"\n", i->arg1);
                else printf("PUSH_INT %d\n", i->int_arg);
                break;
            case OP_LOAD_VAR:    printf("LOAD_VAR %s\n", i->arg1); break;
            case OP_STORE_VAR:   printf("STORE_VAR %s\n", i->arg1); break;
            case OP_ADD:         printf("ADD\n"); break;
            case OP_SUB:         printf("SUB\n"); break;
            case OP_MUL:         printf("MUL\n"); break;
            case OP_DIV:         printf("DIV\n"); break;
            case OP_EQ:          printf("CMP_EQ\n"); break;
            case OP_NEQ:         printf("CMP_NEQ\n"); break;
            case OP_LT:          printf("CMP_LT\n"); break;
            case OP_LTE:         printf("CMP_LTE\n"); break;
            case OP_GT:          printf("CMP_GT\n"); break;
            case OP_GTE:         printf("CMP_GTE\n"); break;
            case OP_AND:         printf("AND\n"); break;
            case OP_OR:          printf("OR\n"); break;
            case OP_NOT:         printf("NOT\n"); break;
            case OP_JUMP:        printf("JUMP %s\n", i->arg1); break;
            case OP_JUMP_IF_FALSE: printf("JUMP_IF_FALSE %s\n", i->arg1); break;
            case OP_CALL:        printf("CALL %s (args: %d)\n", i->arg1, i->int_arg); break;
            case OP_RET:         printf("RET\n"); break;
            case OP_PRINT:       printf("PRINT\n"); break;
            case OP_ALLOC_STRUCT: printf("ALLOC_STRUCT %s\n", i->arg1); break;
            case OP_GET_FIELD:   printf("GET_FIELD .%s\n", i->arg1); break;
            case OP_SET_FIELD:   printf("SET_FIELD .%s\n", i->arg1); break;
        }
    }
    printf("==========================================\n\n");
}

// VM Execution Engine
static VarEntry* find_var(CallFrame *frame, const char *name) {
    CallFrame *curr = frame;
    while (curr) {
        for (VarEntry *v = curr->locals; v; v = v->next) {
            if (strcmp(v->name, name) == 0) return v;
        }
        curr = curr->parent_frame; // Lexical parent for nested functions
    }
    return NULL;
}

static void set_var(CallFrame *frame, const char *name, VMVal val) {
    VarEntry *existing = find_var(frame, name);
    if (existing) {
        existing->val = val;
        return;
    }
    VarEntry *n = (VarEntry*)malloc(sizeof(VarEntry));
    strncpy(n->name, name, 63);
    n->val = val;
    n->next = frame->locals;
    frame->locals = n;
}

static int find_label_ip(BytecodeProgram *code, const char *label) {
    for (int ip = 0; ip < code->count; ip++) {
        if (code->instructions[ip].op == OP_HALT && strcmp(code->instructions[ip].arg1, label) == 0) {
            return ip;
        }
    }
    return -1;
}

int vm_execute(BytecodeProgram *code) {
    if (!code || code->count == 0) return 0;

    // Find main function entry
    int ip = find_label_ip(code, "main");
    if (ip == -1) {
        // Run from start if no main
        ip = 0;
    }

    VMVal stack[1024];
    int sp = 0;

    CallFrame *call_stack[256];
    int fp = 0;

    CallFrame *current_frame = (CallFrame*)calloc(1, sizeof(CallFrame));
    strcpy(current_frame->func_name, "main");
    current_frame->return_ip = -1;
    call_stack[fp] = current_frame;

    while (ip >= 0 && ip < code->count) {
        BytecodeInstr *instr = &code->instructions[ip++];

        switch (instr->op) {
            case OP_HALT:
                if (strcmp(instr->arg2, "LABEL") != 0 && strcmp(instr->arg2, "FUNC_LABEL") != 0) {
                    return 0; // Natural termination
                }
                break;

            case OP_PUSH_CONST: {
                VMVal v;
                memset(&v, 0, sizeof(v));
                if (strcmp(instr->arg2, "str") == 0) {
                    v.type = VAL_STR;
                    strncpy(v.str_val, instr->arg1, 127);
                } else {
                    v.type = VAL_INT;
                    v.int_val = instr->int_arg;
                }
                stack[sp++] = v;
                break;
            }

            case OP_LOAD_VAR: {
                VarEntry *v = find_var(current_frame, instr->arg1);
                if (v) {
                    stack[sp++] = v->val;
                } else {
                    VMVal zero = { VAL_INT, 0, "", NULL };
                    stack[sp++] = zero;
                }
                break;
            }

            case OP_STORE_VAR: {
                if (sp > 0) {
                    VMVal v = stack[--sp];
                    set_var(current_frame, instr->arg1, v);
                }
                break;
            }

            case OP_ADD: {
                VMVal b = stack[--sp];
                VMVal a = stack[--sp];
                VMVal res = { VAL_INT, a.int_val + b.int_val, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_SUB: {
                VMVal b = stack[--sp];
                VMVal a = stack[--sp];
                VMVal res = { VAL_INT, a.int_val - b.int_val, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_MUL: {
                VMVal b = stack[--sp];
                VMVal a = stack[--sp];
                VMVal res = { VAL_INT, a.int_val * b.int_val, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_DIV: {
                VMVal b = stack[--sp];
                VMVal a = stack[--sp];
                int r = (b.int_val != 0) ? (a.int_val / b.int_val) : 0;
                VMVal res = { VAL_INT, r, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_EQ: {
                VMVal b = stack[--sp];
                VMVal a = stack[--sp];
                int eq = (a.type == VAL_STR) ? (strcmp(a.str_val, b.str_val) == 0) : (a.int_val == b.int_val);
                VMVal res = { VAL_INT, eq, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_NEQ: {
                VMVal b = stack[--sp];
                VMVal a = stack[--sp];
                int eq = (a.type == VAL_STR) ? (strcmp(a.str_val, b.str_val) == 0) : (a.int_val == b.int_val);
                VMVal res = { VAL_INT, !eq, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_LT: {
                VMVal b = stack[--sp];
                VMVal a = stack[--sp];
                VMVal res = { VAL_INT, a.int_val < b.int_val, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_LTE: {
                VMVal b = stack[--sp];
                VMVal a = stack[--sp];
                VMVal res = { VAL_INT, a.int_val <= b.int_val, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_GT: {
                VMVal b = stack[--sp];
                VMVal a = stack[--sp];
                VMVal res = { VAL_INT, a.int_val > b.int_val, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_GTE: {
                VMVal b = stack[--sp];
                VMVal a = stack[--sp];
                VMVal res = { VAL_INT, a.int_val >= b.int_val, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_AND: {
                VMVal b = stack[--sp];
                VMVal a = stack[--sp];
                VMVal res = { VAL_INT, a.int_val && b.int_val, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_OR: {
                VMVal b = stack[--sp];
                VMVal a = stack[--sp];
                VMVal res = { VAL_INT, a.int_val || b.int_val, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_NOT: {
                VMVal a = stack[--sp];
                VMVal res = { VAL_INT, !a.int_val, "", NULL };
                stack[sp++] = res;
                break;
            }

            case OP_JUMP: {
                int target = find_label_ip(code, instr->arg1);
                if (target != -1) ip = target;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                VMVal cond = stack[--sp];
                if (!cond.int_val) {
                    int target = find_label_ip(code, instr->arg1);
                    if (target != -1) ip = target;
                }
                break;
            }

            case OP_CALL: {
                int target = find_label_ip(code, instr->arg1);
                if (target == -1) {
                    fprintf(stderr, "VM runtime error: target function '%s' not found\n", instr->arg1);
                    return 1;
                }
                CallFrame *new_frame = (CallFrame*)calloc(1, sizeof(CallFrame));
                strncpy(new_frame->func_name, instr->arg1, 63);
                new_frame->return_ip = ip;
                new_frame->parent_frame = current_frame; // Lexical static link

                call_stack[++fp] = new_frame;
                current_frame = new_frame;
                ip = target;
                break;
            }

            case OP_RET: {
                int ret_ip = current_frame->return_ip;
                // Pop frame
                if (fp > 0) {
                    fp--;
                    current_frame = call_stack[fp];
                    ip = ret_ip;
                } else {
                    return 0; // Returned from main
                }
                break;
            }

            case OP_PRINT: {
                VMVal v = stack[--sp];
                if (v.type == VAL_INT) {
                    printf("%d\n", v.int_val);
                } else if (v.type == VAL_STR) {
                    printf("%s\n", v.str_val);
                } else if (v.type == VAL_OBJ && v.obj_val) {
                    printf("<struct %s at %p>\n", v.obj_val->struct_name, (void*)v.obj_val);
                }
                break;
            }

            case OP_ALLOC_STRUCT: {
                StructObj *obj = (StructObj*)calloc(1, sizeof(StructObj));
                strncpy(obj->struct_name, instr->arg1, 63);
                VMVal v;
                v.type = VAL_OBJ;
                v.obj_val = obj;
                v.int_val = 0;
                stack[sp++] = v;
                break;
            }

            case OP_GET_FIELD: {
                VMVal obj_val = stack[--sp];
                if (obj_val.type == VAL_OBJ && obj_val.obj_val) {
                    int found = 0;
                    for (StructFieldVal *f = obj_val.obj_val->fields; f; f = f->next) {
                        if (strcmp(f->name, instr->arg1) == 0) {
                            VMVal f_val = { VAL_INT, f->int_val, "", NULL };
                            if (strlen(f->str_val) > 0) {
                                f_val.type = VAL_STR;
                                strcpy(f_val.str_val, f->str_val);
                            }
                            stack[sp++] = f_val;
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        VMVal def = { VAL_INT, 0, "", NULL };
                        stack[sp++] = def;
                    }
                } else {
                    VMVal def = { VAL_INT, 0, "", NULL };
                    stack[sp++] = def;
                }
                break;
            }

            case OP_SET_FIELD: {
                VMVal obj_val = stack[--sp];
                VMVal val = stack[--sp];
                if (obj_val.type == VAL_OBJ && obj_val.obj_val) {
                    StructFieldVal *target_f = NULL;
                    for (StructFieldVal *f = obj_val.obj_val->fields; f; f = f->next) {
                        if (strcmp(f->name, instr->arg1) == 0) {
                            target_f = f;
                            break;
                        }
                    }
                    if (!target_f) {
                        target_f = (StructFieldVal*)calloc(1, sizeof(StructFieldVal));
                        strncpy(target_f->name, instr->arg1, 63);
                        target_f->next = obj_val.obj_val->fields;
                        obj_val.obj_val->fields = target_f;
                    }
                    if (val.type == VAL_STR) {
                        strncpy(target_f->str_val, val.str_val, 127);
                    } else {
                        target_f->int_val = val.int_val;
                    }
                }
                break;
            }
        }
    }

    return 0;
}

void vm_free(BytecodeProgram *code) {
    if (!code) return;
    if (code->instructions) free(code->instructions);
    free(code);
}
