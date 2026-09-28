#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "optimizer.h"

// Check if string is a pure integer literal
static int is_int_const(const char *str, int *out) {
    if (!str || *str == '\0') return 0;
    int i = 0;
    if (str[0] == '-' && isdigit(str[1])) i = 1;
    for (; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) return 0;
    }
    *out = atoi(str);
    return 1;
}

// Pass 1: Constant Folding
int optimize_constant_folding(TACProgram *prog) {
    int folded = 0;
    if (!prog) return 0;

    for (TACInstr *i = prog->head; i; i = i->next) {
        int v1, v2;
        if (is_int_const(i->arg1, &v1) && is_int_const(i->arg2, &v2)) {
            int result = 0;
            int can_fold = 1;

            switch (i->op) {
                case TAC_ADD: result = v1 + v2; break;
                case TAC_SUB: result = v1 - v2; break;
                case TAC_MUL: result = v1 * v2; break;
                case TAC_DIV:
                    if (v2 != 0) result = v1 / v2;
                    else can_fold = 0;
                    break;
                case TAC_EQ:  result = (v1 == v2); break;
                case TAC_NEQ: result = (v1 != v2); break;
                case TAC_LT:  result = (v1 < v2); break;
                case TAC_LTE: result = (v1 <= v2); break;
                case TAC_GT:  result = (v1 > v2); break;
                case TAC_GTE: result = (v1 >= v2); break;
                case TAC_AND: result = (v1 && v2); break;
                case TAC_OR:  result = (v1 || v2); break;
                default: can_fold = 0; break;
            }

            if (can_fold) {
                i->op = TAC_ASSIGN;
                snprintf(i->arg1, sizeof(i->arg1), "%d", result);
                i->arg2[0] = '\0';
                folded++;
            }
        }
    }
    return folded;
}

// Pass 2: Dead Code Elimination (Unreachable instructions after GOTO / RETURN)
int optimize_dead_code_elimination(TACProgram *prog) {
    int removed = 0;
    if (!prog) return 0;

    TACInstr *curr = prog->head;
    while (curr) {
        if (curr->op == TAC_GOTO || curr->op == TAC_RETURN) {
            // All instructions until the next label or function boundary are unreachable
            TACInstr *dead = curr->next;
            while (dead && dead->op != TAC_LABEL && dead->op != TAC_FUNC_BEGIN && dead->op != TAC_FUNC_END) {
                TACInstr *to_free = dead;
                dead = dead->next;
                curr->next = dead;
                free(to_free);
                removed++;
            }
        }
        curr = curr->next;
    }

    return removed;
}

void optimize_tac(TACProgram *prog) {
    int total_folded = 0;
    int total_dce = 0;

    while (1) {
        int cf = optimize_constant_folding(prog);
        int dce = optimize_dead_code_elimination(prog);
        total_folded += cf;
        total_dce += dce;
        if (cf == 0 && dce == 0) break;
    }

    printf("  [Optimizer summary: %d constant(s) folded, %d dead instruction(s) removed]\n",
           total_folded, total_dce);
}
