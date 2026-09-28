#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tac.h"

TACProgram* tac_create_program(void) {
    TACProgram *p = (TACProgram*)calloc(1, sizeof(TACProgram));
    p->temp_count = 1;
    p->label_count = 1;
    return p;
}

void tac_append(TACProgram *prog, TACOp op, const char *res, const char *a1, const char *a2) {
    TACInstr *instr = (TACInstr*)calloc(1, sizeof(TACInstr));
    instr->op = op;
    if (res) strncpy(instr->result, res, 63);
    if (a1) strncpy(instr->arg1, a1, 63);
    if (a2) strncpy(instr->arg2, a2, 63);

    if (!prog->head) {
        prog->head = instr;
        prog->tail = instr;
    } else {
        prog->tail->next = instr;
        prog->tail = instr;
    }
}

char* tac_new_temp(TACProgram *prog) {
    static char buf[64];
    snprintf(buf, sizeof(buf), "t%d", prog->temp_count++);
    return strdup(buf);
}

char* tac_new_label(TACProgram *prog) {
    static char buf[64];
    snprintf(buf, sizeof(buf), "L%d", prog->label_count++);
    return strdup(buf);
}

// Forward declarations for code generation
static char* gen_expr(TACProgram *prog, ASTNode *expr);
static void gen_stmt(TACProgram *prog, ASTNode *stmt);
static void gen_func(TACProgram *prog, ASTNode *func);

static char* gen_expr(TACProgram *prog, ASTNode *expr) {
    if (!expr) return "";

    switch (expr->type) {
        case AST_LITERAL_INT: {
            char buf[64];
            snprintf(buf, sizeof(buf), "%d", expr->as.lit_int.int_val);
            return strdup(buf);
        }

        case AST_LITERAL_STRING: {
            char buf[128];
            snprintf(buf, sizeof(buf), "\"%s\"", expr->as.lit_str.str_val);
            return strdup(buf);
        }

        case AST_LITERAL_BOOL: {
            return strdup(expr->as.lit_bool.bool_val ? "1" : "0");
        }

        case AST_VAR_EXPR: {
            return strdup(expr->as.var_expr.name);
        }

        case AST_NEW_EXPR: {
            char *res = tac_new_temp(prog);
            tac_append(prog, TAC_NEW, res, expr->as.new_expr.struct_name, NULL);
            return res;
        }

        case AST_FIELD_ACCESS_EXPR: {
            char *obj = gen_expr(prog, expr->as.field_access.object);
            char *res = tac_new_temp(prog);
            tac_append(prog, TAC_GET_FIELD, res, obj, expr->as.field_access.field_name);
            return res;
        }

        case AST_BINARY_EXPR: {
            char *left = gen_expr(prog, expr->as.binary_expr.left);
            char *right = gen_expr(prog, expr->as.binary_expr.right);
            char *res = tac_new_temp(prog);

            TACOp op = TAC_NOP;
            switch (expr->as.binary_expr.op) {
                case TOKEN_PLUS: op = TAC_ADD; break;
                case TOKEN_MINUS: op = TAC_SUB; break;
                case TOKEN_STAR: op = TAC_MUL; break;
                case TOKEN_SLASH: op = TAC_DIV; break;
                case TOKEN_EQ: op = TAC_EQ; break;
                case TOKEN_NEQ: op = TAC_NEQ; break;
                case TOKEN_LT: op = TAC_LT; break;
                case TOKEN_LTE: op = TAC_LTE; break;
                case TOKEN_GT: op = TAC_GT; break;
                case TOKEN_GTE: op = TAC_GTE; break;
                case TOKEN_AND: op = TAC_AND; break;
                case TOKEN_OR: op = TAC_OR; break;
                default: break;
            }

            tac_append(prog, op, res, left, right);
            return res;
        }

        case AST_UNARY_EXPR: {
            char *operand = gen_expr(prog, expr->as.unary_expr.operand);
            char *res = tac_new_temp(prog);
            if (expr->as.unary_expr.op == TOKEN_MINUS) {
                tac_append(prog, TAC_NEG, res, operand, NULL);
            } else if (expr->as.unary_expr.op == TOKEN_NOT) {
                tac_append(prog, TAC_NOT, res, operand, NULL);
            }
            return res;
        }

        case AST_CALL_EXPR: {
            // Push arguments
            int arg_count = 0;
            for (ASTNodeList *l = expr->as.call_expr.args; l; l = l->next) {
                char *arg_val = gen_expr(prog, l->node);
                tac_append(prog, TAC_PARAM, NULL, arg_val, NULL);
                arg_count++;
            }
            char *res = tac_new_temp(prog);
            char cnt_str[16];
            snprintf(cnt_str, sizeof(cnt_str), "%d", arg_count);
            tac_append(prog, TAC_CALL, res, expr->as.call_expr.func_name, cnt_str);
            return res;
        }

        default:
            return "";
    }
}

static void gen_stmt(TACProgram *prog, ASTNode *stmt) {
    if (!stmt) return;

    switch (stmt->type) {
        case AST_VAR_DECL: {
            if (stmt->as.var_decl.init_expr) {
                char *val = gen_expr(prog, stmt->as.var_decl.init_expr);
                tac_append(prog, TAC_ASSIGN, stmt->as.var_decl.name, val, NULL);
            }
            break;
        }

        case AST_ASSIGN_STMT: {
            ASTNode *tgt = stmt->as.assign_stmt.target;
            char *val = gen_expr(prog, stmt->as.assign_stmt.value);

            if (tgt->type == AST_VAR_EXPR) {
                tac_append(prog, TAC_ASSIGN, tgt->as.var_expr.name, val, NULL);
            } else if (tgt->type == AST_FIELD_ACCESS_EXPR) {
                char *obj = gen_expr(prog, tgt->as.field_access.object);
                tac_append(prog, TAC_SET_FIELD, val, obj, tgt->as.field_access.field_name);
            }
            break;
        }

        case AST_IF_STMT: {
            char *cond = gen_expr(prog, stmt->as.if_stmt.condition);
            char *lbl_else = tac_new_label(prog);
            char *lbl_end = tac_new_label(prog);

            if (stmt->as.if_stmt.else_branch) {
                tac_append(prog, TAC_IF_FALSE, NULL, cond, lbl_else);
                gen_stmt(prog, stmt->as.if_stmt.then_branch);
                tac_append(prog, TAC_GOTO, NULL, lbl_end, NULL);
                tac_append(prog, TAC_LABEL, lbl_else, NULL, NULL);
                gen_stmt(prog, stmt->as.if_stmt.else_branch);
                tac_append(prog, TAC_LABEL, lbl_end, NULL, NULL);
            } else {
                tac_append(prog, TAC_IF_FALSE, NULL, cond, lbl_end);
                gen_stmt(prog, stmt->as.if_stmt.then_branch);
                tac_append(prog, TAC_LABEL, lbl_end, NULL, NULL);
            }
            break;
        }

        case AST_PRINT_STMT: {
            char *val = gen_expr(prog, stmt->as.print_stmt.expr);
            tac_append(prog, TAC_PRINT, NULL, val, NULL);
            break;
        }

        case AST_RETURN_STMT: {
            if (stmt->as.return_stmt.expr) {
                char *val = gen_expr(prog, stmt->as.return_stmt.expr);
                tac_append(prog, TAC_RETURN, NULL, val, NULL);
            } else {
                tac_append(prog, TAC_RETURN, NULL, NULL, NULL);
            }
            break;
        }

        case AST_EXPR_STMT: {
            gen_expr(prog, stmt->as.expr_stmt.expr);
            break;
        }

        case AST_BLOCK: {
            for (ASTNodeList *l = stmt->as.block.statements; l; l = l->next) {
                gen_stmt(prog, l->node);
            }
            break;
        }

        case AST_FUNC_DECL: {
            char *skip = tac_new_label(prog);
            tac_append(prog, TAC_GOTO, NULL, skip, NULL);
            gen_func(prog, stmt);
            tac_append(prog, TAC_LABEL, skip, NULL, NULL);
            break;
        }

        default:
            break;
    }
}

static void gen_func(TACProgram *prog, ASTNode *func) {
    tac_append(prog, TAC_FUNC_BEGIN, func->as.func_decl.name, NULL, NULL);

    // Pop parameters in reverse order (stack discipline)
    int p_count = 0;
    for (ASTNodeList *p = func->as.func_decl.params; p; p = p->next) p_count++;
    if (p_count > 0) {
        char **pnames = (char**)malloc(sizeof(char*) * p_count);
        int idx = 0;
        for (ASTNodeList *p = func->as.func_decl.params; p; p = p->next) {
            pnames[idx++] = p->node->as.var_decl.name;
        }
        for (int i = p_count - 1; i >= 0; i--) {
            tac_append(prog, TAC_PARAM_POP, pnames[i], NULL, NULL);
        }
        free(pnames);
    }

    // Generate statements (any nested functions will be generated with their skip labels)
    if (func->as.func_decl.body) {
        for (ASTNodeList *l = func->as.func_decl.body->as.block.statements; l; l = l->next) {
            gen_stmt(prog, l->node);
        }
    }

    tac_append(prog, TAC_FUNC_END, func->as.func_decl.name, NULL, NULL);
}

TACProgram* tac_generate(ASTNode *root) {
    if (!root) return NULL;
    TACProgram *prog = tac_create_program();

    for (ASTNodeList *l = root->as.program.statements; l; l = l->next) {
        ASTNode *node = l->node;
        if (node->type == AST_FUNC_DECL) {
            gen_func(prog, node);
        } else if (node->type == AST_VAR_DECL) {
            gen_stmt(prog, node);
        }
    }

    return prog;
}

void tac_print(TACProgram *prog) {
    if (!prog) return;

    for (TACInstr *i = prog->head; i; i = i->next) {
        switch (i->op) {
            case TAC_FUNC_BEGIN:
                printf("\n--- FUNCTION BEGIN: %s ---\n", i->result);
                break;
            case TAC_FUNC_END:
                printf("--- FUNCTION END: %s ---\n", i->result);
                break;
            case TAC_LABEL:
                printf("%s:\n", i->result);
                break;
            case TAC_ASSIGN:
                printf("    %s = %s\n", i->result, i->arg1);
                break;
            case TAC_ADD:
                printf("    %s = %s + %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_SUB:
                printf("    %s = %s - %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_MUL:
                printf("    %s = %s * %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_DIV:
                printf("    %s = %s / %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_EQ:
                printf("    %s = %s == %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_NEQ:
                printf("    %s = %s != %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_LT:
                printf("    %s = %s < %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_LTE:
                printf("    %s = %s <= %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_GT:
                printf("    %s = %s > %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_GTE:
                printf("    %s = %s >= %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_AND:
                printf("    %s = %s && %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_OR:
                printf("    %s = %s || %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_NOT:
                printf("    %s = !%s\n", i->result, i->arg1);
                break;
            case TAC_NEG:
                printf("    %s = -%s\n", i->result, i->arg1);
                break;
            case TAC_IF_FALSE:
                printf("    if_false %s goto %s\n", i->arg1, i->arg2);
                break;
            case TAC_GOTO:
                printf("    goto %s\n", i->arg1);
                break;
            case TAC_PARAM:
                printf("    param %s\n", i->arg1);
                break;
            case TAC_PARAM_POP:
                printf("    pop_param %s\n", i->result);
                break;
            case TAC_CALL:
                printf("    %s = call %s, %s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_RETURN:
                if (strlen(i->arg1) > 0) printf("    return %s\n", i->arg1);
                else printf("    return\n");
                break;
            case TAC_PRINT:
                printf("    print %s\n", i->arg1);
                break;
            case TAC_NEW:
                printf("    %s = new %s\n", i->result, i->arg1);
                break;
            case TAC_GET_FIELD:
                printf("    %s = %s.%s\n", i->result, i->arg1, i->arg2);
                break;
            case TAC_SET_FIELD:
                printf("    %s.%s = %s\n", i->arg1, i->arg2, i->result);
                break;
            default:
                break;
        }
    }
}

void tac_free(TACProgram *prog) {
    if (!prog) return;
    TACInstr *curr = prog->head;
    while (curr) {
        TACInstr *next = curr->next;
        free(curr);
        curr = next;
    }
    free(prog);
}
