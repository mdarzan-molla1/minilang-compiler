#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

static int error_count = 0;
static Scope *global_scope = NULL;
static const char *current_func_ret_type = NULL;

int semantic_error_count(void) {
    return error_count;
}

static void sem_error(int line, int col, const char *msg) {
    error_count++;
    fprintf(stderr, "Semantic Error [Line %d, Col %d]: %s\n", line, col, msg);
}

// Forward declarations
static const char* check_expression(ASTNode *expr, Scope *scope);
static void check_statement(ASTNode *stmt, Scope *scope);
static void check_func_decl(ASTNode *func, Scope *parent_scope);

// Check if two type names are compatible
static int type_equals(const char *t1, const char *t2) {
    if (!t1 || !t2) return 0;
    if (strcmp(t1, "unknown") == 0 || strcmp(t2, "unknown") == 0) return 1; // don't cascade
    return strcmp(t1, t2) == 0;
}

// Semantic Analysis: Check Expression and return its evaluated type name
static const char* check_expression(ASTNode *expr, Scope *scope) {
    if (!expr) return "diov42";

    switch (expr->type) {
        case AST_LITERAL_INT:
            strcpy(expr->type_name, "tni42");
            return "tni42";

        case AST_LITERAL_STRING:
            strcpy(expr->type_name, "gnirts42");
            return "gnirts42";

        case AST_LITERAL_BOOL:
            strcpy(expr->type_name, "loob42");
            return "loob42";

        case AST_NEW_EXPR: {
            Symbol *struct_sym = scope_lookup(scope, expr->as.new_expr.struct_name);
            if (!struct_sym || struct_sym->kind != SYM_STRUCT) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Undefined struct '%s' in wen42 instantiation", expr->as.new_expr.struct_name);
                sem_error(expr->line, expr->col, buf);
                strcpy(expr->type_name, "unknown");
                return "unknown";
            }
            strcpy(expr->type_name, expr->as.new_expr.struct_name);
            return expr->type_name;
        }

        case AST_VAR_EXPR: {
            // [Semantic Error 1]: Undeclared identifier
            Symbol *sym = scope_lookup(scope, expr->as.var_expr.name);
            if (!sym) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Undeclared identifier '%s'", expr->as.var_expr.name);
                sem_error(expr->line, expr->col, buf);
                strcpy(expr->type_name, "unknown");
                return "unknown";
            }
            strcpy(expr->type_name, sym->type_name);
            return expr->type_name;
        }

        case AST_FIELD_ACCESS_EXPR: {
            // [Semantic Error 4]: Invalid struct field access
            const char *obj_type = check_expression(expr->as.field_access.object, scope);
            if (strcmp(obj_type, "unknown") == 0) {
                strcpy(expr->type_name, "unknown");
                return "unknown";
            }

            Symbol *struct_sym = scope_lookup(scope, obj_type);
            if (!struct_sym || struct_sym->kind != SYM_STRUCT) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Cannot access field '%s' on non-struct type '%s'",
                         expr->as.field_access.field_name, obj_type);
                sem_error(expr->line, expr->col, buf);
                strcpy(expr->type_name, "unknown");
                return "unknown";
            }

            // Look up field in struct
            FieldInfo *f = struct_sym->fields;
            while (f) {
                if (strcmp(f->name, expr->as.field_access.field_name) == 0) {
                    strcpy(expr->type_name, f->type_name);
                    return expr->type_name;
                }
                f = f->next;
            }

            char buf[128];
            snprintf(buf, sizeof(buf), "Field '%s' does not exist in struct '%s'",
                     expr->as.field_access.field_name, obj_type);
            sem_error(expr->line, expr->col, buf);
            strcpy(expr->type_name, "unknown");
            return "unknown";
        }

        case AST_CALL_EXPR: {
            Symbol *sym = scope_lookup(scope, expr->as.call_expr.func_name);
            if (!sym || sym->kind != SYM_FUNC) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Undeclared function '%s'", expr->as.call_expr.func_name);
                sem_error(expr->line, expr->col, buf);
                strcpy(expr->type_name, "unknown");
                return "unknown";
            }

            // Check argument count and types
            ParamInfo *p = sym->params;
            ASTNodeList *arg = expr->as.call_expr.args;
            int arg_idx = 1;
            while (p && arg) {
                const char *arg_type = check_expression(arg->node, scope);
                if (!type_equals(p->type_name, arg_type)) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Argument %d of '%s' expects type '%s', got '%s'",
                             arg_idx, sym->name, p->type_name, arg_type);
                    sem_error(arg->node->line, arg->node->col, buf);
                }
                p = p->next;
                arg = arg->next;
                arg_idx++;
            }
            if (p != NULL || arg != NULL) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Function '%s' argument count mismatch", sym->name);
                sem_error(expr->line, expr->col, buf);
            }

            strcpy(expr->type_name, sym->type_name);
            return expr->type_name;
        }

        case AST_BINARY_EXPR: {
            const char *t_left = check_expression(expr->as.binary_expr.left, scope);
            const char *t_right = check_expression(expr->as.binary_expr.right, scope);
            TokenType op = expr->as.binary_expr.op;

            // Arithmetic: + - * /
            if (op == TOKEN_PLUS || op == TOKEN_MINUS || op == TOKEN_STAR || op == TOKEN_SLASH) {
                if (!type_equals(t_left, "tni42") || !type_equals(t_right, "tni42")) {
                    // [Semantic Error 3]: Type mismatch in binary arithmetic
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Arithmetic operator '%s' requires operands of type 'tni42', got '%s' and '%s'",
                             token_type_name(op), t_left, t_right);
                    sem_error(expr->line, expr->col, buf);
                }
                strcpy(expr->type_name, "tni42");
                return "tni42";
            }

            // Relational: < <= > >=
            if (op == TOKEN_LT || op == TOKEN_LTE || op == TOKEN_GT || op == TOKEN_GTE) {
                if (!type_equals(t_left, "tni42") || !type_equals(t_right, "tni42")) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Relational operator requires operands of type 'tni42', got '%s' and '%s'",
                             t_left, t_right);
                    sem_error(expr->line, expr->col, buf);
                }
                strcpy(expr->type_name, "loob42");
                return "loob42";
            }

            // Equality: == !=
            if (op == TOKEN_EQ || op == TOKEN_NEQ) {
                if (!type_equals(t_left, t_right)) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Equality comparison type mismatch between '%s' and '%s'",
                             t_left, t_right);
                    sem_error(expr->line, expr->col, buf);
                }
                strcpy(expr->type_name, "loob42");
                return "loob42";
            }

            // Logical: && ||
            if (op == TOKEN_AND || op == TOKEN_OR) {
                if (!type_equals(t_left, "loob42") || !type_equals(t_right, "loob42")) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Logical operator requires boolean operands, got '%s' and '%s'",
                             t_left, t_right);
                    sem_error(expr->line, expr->col, buf);
                }
                strcpy(expr->type_name, "loob42");
                return "loob42";
            }

            strcpy(expr->type_name, "unknown");
            return "unknown";
        }

        case AST_UNARY_EXPR: {
            const char *sub = check_expression(expr->as.unary_expr.operand, scope);
            TokenType op = expr->as.unary_expr.op;
            if (op == TOKEN_MINUS) {
                if (!type_equals(sub, "tni42")) {
                    sem_error(expr->line, expr->col, "Unary '-' requires 'tni42' operand");
                }
                strcpy(expr->type_name, "tni42");
                return "tni42";
            } else if (op == TOKEN_NOT) {
                if (!type_equals(sub, "loob42")) {
                    sem_error(expr->line, expr->col, "Unary '!' requires 'loob42' operand");
                }
                strcpy(expr->type_name, "loob42");
                return "loob42";
            }
            strcpy(expr->type_name, "unknown");
            return "unknown";
        }

        default:
            return "diov42";
    }
}

static void check_statement(ASTNode *stmt, Scope *scope) {
    if (!stmt) return;

    switch (stmt->type) {
        case AST_VAR_DECL: {
            // [Semantic Error 2]: Duplicate declaration in current scope
            Symbol *sym = symbol_new_var(stmt->as.var_decl.name, stmt->type_name, stmt->line, stmt->col);
            if (!scope_insert(scope, sym)) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Redeclaration of variable '%s' in the same scope", stmt->as.var_decl.name);
                sem_error(stmt->line, stmt->col, buf);
                free(sym);
            }

            if (stmt->as.var_decl.init_expr) {
                const char *val_type = check_expression(stmt->as.var_decl.init_expr, scope);
                // [Semantic Error 3]: Type mismatch in variable initialization
                if (!type_equals(stmt->type_name, val_type)) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Type mismatch in variable initialization: cannot assign '%s' to '%s'",
                             val_type, stmt->type_name);
                    sem_error(stmt->line, stmt->col, buf);
                }
            }
            break;
        }

        case AST_ASSIGN_STMT: {
            const char *target_type = check_expression(stmt->as.assign_stmt.target, scope);
            const char *val_type = check_expression(stmt->as.assign_stmt.value, scope);
            // [Semantic Error 3]: Type mismatch in assignment
            if (!type_equals(target_type, val_type)) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Type mismatch in assignment: cannot assign '%s' to '%s'",
                         val_type, target_type);
                sem_error(stmt->line, stmt->col, buf);
            }
            break;
        }

        case AST_IF_STMT: {
            const char *cond_type = check_expression(stmt->as.if_stmt.condition, scope);
            if (!type_equals(cond_type, "loob42")) {
                sem_error(stmt->line, stmt->col, "If condition must be a boolean ('loob42')");
            }
            check_statement(stmt->as.if_stmt.then_branch, scope);
            if (stmt->as.if_stmt.else_branch) {
                check_statement(stmt->as.if_stmt.else_branch, scope);
            }
            break;
        }

        case AST_PRINT_STMT: {
            check_expression(stmt->as.print_stmt.expr, scope);
            break;
        }

        case AST_RETURN_STMT: {
            // [Semantic Error 5]: Return type mismatch
            if (stmt->as.return_stmt.expr) {
                const char *ret_expr_type = check_expression(stmt->as.return_stmt.expr, scope);
                if (type_equals(current_func_ret_type, "diov42")) {
                    sem_error(stmt->line, stmt->col, "Cannot return a value from a 'diov42' function");
                } else if (!type_equals(current_func_ret_type, ret_expr_type)) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Return type mismatch: function returns '%s', but return expression is '%s'",
                             current_func_ret_type, ret_expr_type);
                    sem_error(stmt->line, stmt->col, buf);
                }
            } else {
                if (current_func_ret_type && !type_equals(current_func_ret_type, "diov42")) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Empty return in non-void function (expected '%s')", current_func_ret_type);
                    sem_error(stmt->line, stmt->col, buf);
                }
            }
            break;
        }

        case AST_EXPR_STMT: {
            check_expression(stmt->as.expr_stmt.expr, scope);
            break;
        }

        case AST_BLOCK: {
            // Nested block scope
            Scope *block_scope = scope_create(scope);
            for (ASTNodeList *l = stmt->as.block.statements; l; l = l->next) {
                check_statement(l->node, block_scope);
            }
            // Note: in a production compiler we might keep scopes alive for code gen,
            // but for simplicity we maintain static links.
            break;
        }

        case AST_FUNC_DECL: {
            check_func_decl(stmt, scope);
            break;
        }

        default:
            break;
    }
}

static void check_func_decl(ASTNode *func, Scope *parent_scope) {
    // 1. Insert function into parent scope
    Symbol *fn_sym = symbol_new_func(func->as.func_decl.name, func->type_name, func->line, func->col);

    // Register params
    ParamInfo *p_head = NULL;
    ParamInfo *p_tail = NULL;
    int p_cnt = 0;
    for (ASTNodeList *p = func->as.func_decl.params; p; p = p->next) {
        ParamInfo *pi = (ParamInfo*)malloc(sizeof(ParamInfo));
        strcpy(pi->name, p->node->as.var_decl.name);
        strcpy(pi->type_name, p->node->type_name);
        pi->next = NULL;
        if (!p_head) p_head = pi;
        else p_tail->next = pi;
        p_tail = pi;
        p_cnt++;
    }
    fn_sym->params = p_head;
    fn_sym->param_count = p_cnt;

    // [Semantic Error 2]: Duplicate function declaration in the same scope
    if (!scope_insert(parent_scope, fn_sym)) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Redeclaration of function '%s' in the same scope", func->as.func_decl.name);
        sem_error(func->line, func->col, buf);
    }

    // 2. Create function body scope (child of parent_scope -> static scoping!)
    Scope *func_scope = scope_create(parent_scope);

    // Insert parameters into function body scope
    for (ParamInfo *pi = fn_sym->params; pi; pi = pi->next) {
        Symbol *p_var = symbol_new_var(pi->name, pi->type_name, func->line, func->col);
        if (!scope_insert(func_scope, p_var)) {
            char buf[128];
            snprintf(buf, sizeof(buf), "Duplicate parameter name '%s' in function '%s'", pi->name, func->as.func_decl.name);
            sem_error(func->line, func->col, buf);
        }
    }

    // Check body statements (nested functions will naturally be checked in order)

    // Check body statements
    const char *prev_ret = current_func_ret_type;
    current_func_ret_type = func->type_name;

    if (func->as.func_decl.body) {
        for (ASTNodeList *s = func->as.func_decl.body->as.block.statements; s; s = s->next) {
            check_statement(s->node, func_scope);
        }
    }

    current_func_ret_type = prev_ret;
}

int semantic_analyze(ASTNode *root) {
    error_count = 0;
    if (!root || root->type != AST_PROGRAM) return 0;

    global_scope = scope_create(NULL);

    // Phase 1: Register all Structs and global declarations
    for (ASTNodeList *l = root->as.program.statements; l; l = l->next) {
        ASTNode *node = l->node;
        if (node->type == AST_STRUCT_DECL) {
            Symbol *st_sym = symbol_new_struct(node->as.struct_decl.name, node->line, node->col);
            FieldInfo *f_head = NULL;
            FieldInfo *f_tail = NULL;

            for (ASTNodeList *fl = node->as.struct_decl.fields; fl; fl = fl->next) {
                FieldInfo *fi = (FieldInfo*)malloc(sizeof(FieldInfo));
                strcpy(fi->name, fl->node->as.var_decl.name);
                strcpy(fi->type_name, fl->node->type_name);
                fi->next = NULL;
                if (!f_head) f_head = fi;
                else f_tail->next = fi;
                f_tail = fi;
            }
            st_sym->fields = f_head;

            if (!scope_insert(global_scope, st_sym)) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Duplicate declaration of struct '%s'", node->as.struct_decl.name);
                sem_error(node->line, node->col, buf);
            }
        }
    }

    // Phase 2: Check all top-level functions and statements
    for (ASTNodeList *l = root->as.program.statements; l; l = l->next) {
        ASTNode *node = l->node;
        if (node->type == AST_FUNC_DECL) {
            check_func_decl(node, global_scope);
        } else if (node->type == AST_VAR_DECL) {
            check_statement(node, global_scope);
        }
    }

    return error_count;
}
