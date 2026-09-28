#ifndef TAC_H
#define TAC_H

#include "ast.h"

typedef enum {
    TAC_NOP = 0,
    TAC_LABEL,
    TAC_ASSIGN,
    TAC_ADD,
    TAC_SUB,
    TAC_MUL,
    TAC_DIV,
    TAC_EQ,
    TAC_NEQ,
    TAC_LT,
    TAC_LTE,
    TAC_GT,
    TAC_GTE,
    TAC_AND,
    TAC_OR,
    TAC_NOT,
    TAC_NEG,
    TAC_IF_FALSE,
    TAC_GOTO,
    TAC_PARAM,
    TAC_PARAM_POP,
    TAC_CALL,
    TAC_RETURN,
    TAC_PRINT,
    TAC_NEW,
    TAC_GET_FIELD,
    TAC_SET_FIELD,
    TAC_FUNC_BEGIN,
    TAC_FUNC_END
} TACOp;

typedef struct TACInstr {
    TACOp op;
    char result[64];
    char arg1[64];
    char arg2[64];
    struct TACInstr *next;
} TACInstr;

typedef struct {
    TACInstr *head;
    TACInstr *tail;
    int temp_count;
    int label_count;
} TACProgram;

TACProgram* tac_create_program(void);
void tac_append(TACProgram *prog, TACOp op, const char *res, const char *a1, const char *a2);
char* tac_new_temp(TACProgram *prog);
char* tac_new_label(TACProgram *prog);

TACProgram* tac_generate(ASTNode *root);
void tac_print(TACProgram *prog);
void tac_free(TACProgram *prog);

#endif // TAC_H
