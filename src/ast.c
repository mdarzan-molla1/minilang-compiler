#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode* ast_new_node(ASTNodeType type, int line, int col) {
    ASTNode *node = (ASTNode*)calloc(1, sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Out of memory allocating AST node\n");
        exit(1);
    }
    node->type = type;
    node->line = line;
    node->col = col;
    return node;
}

ASTNodeList* ast_list_append(ASTNodeList *list, ASTNode *node) {
    ASTNodeList *elem = (ASTNodeList*)malloc(sizeof(ASTNodeList));
    elem->node = node;
    elem->next = NULL;

    if (!list) return elem;

    ASTNodeList *curr = list;
    while (curr->next) {
        curr = curr->next;
    }
    curr->next = elem;
    return list;
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

void ast_print(ASTNode *node, int indent) {
    if (!node) return;

    print_indent(indent);

    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            for (ASTNodeList *l = node->as.program.statements; l; l = l->next) {
                ast_print(l->node, indent + 1);
            }
            break;

        case AST_STRUCT_DECL:
            printf("StructDecl: %s\n", node->as.struct_decl.name);
            for (ASTNodeList *l = node->as.struct_decl.fields; l; l = l->next) {
                ast_print(l->node, indent + 1);
            }
            break;

        case AST_FUNC_DECL:
            printf("FuncDecl: %s() -> %s\n", node->as.func_decl.name, node->type_name);
            if (node->as.func_decl.params) {
                print_indent(indent + 1);
                printf("Parameters:\n");
                for (ASTNodeList *l = node->as.func_decl.params; l; l = l->next) {
                    ast_print(l->node, indent + 2);
                }
            }
            if (node->as.func_decl.nested_funcs) {
                print_indent(indent + 1);
                printf("Nested Functions:\n");
                for (ASTNodeList *l = node->as.func_decl.nested_funcs; l; l = l->next) {
                    ast_print(l->node, indent + 2);
                }
            }
            print_indent(indent + 1);
            printf("Body:\n");
            ast_print(node->as.func_decl.body, indent + 2);
            break;

        case AST_VAR_DECL:
            printf("VarDecl: %s %s", node->type_name, node->as.var_decl.name);
            if (node->as.var_decl.init_expr) {
                printf(" = \n");
                ast_print(node->as.var_decl.init_expr, indent + 1);
            } else {
                printf("\n");
            }
            break;

        case AST_BLOCK:
            printf("Block\n");
            for (ASTNodeList *l = node->as.block.statements; l; l = l->next) {
                ast_print(l->node, indent + 1);
            }
            break;

        case AST_IF_STMT:
            printf("IfStmt\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            ast_print(node->as.if_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("Then:\n");
            ast_print(node->as.if_stmt.then_branch, indent + 2);
            if (node->as.if_stmt.else_branch) {
                print_indent(indent + 1);
                printf("Else:\n");
                ast_print(node->as.if_stmt.else_branch, indent + 2);
            }
            break;

        case AST_ASSIGN_STMT:
            printf("AssignStmt\n");
            print_indent(indent + 1);
            printf("Target:\n");
            ast_print(node->as.assign_stmt.target, indent + 2);
            print_indent(indent + 1);
            printf("Value:\n");
            ast_print(node->as.assign_stmt.value, indent + 2);
            break;

        case AST_PRINT_STMT:
            printf("PrintStmt\n");
            ast_print(node->as.print_stmt.expr, indent + 1);
            break;

        case AST_RETURN_STMT:
            printf("ReturnStmt\n");
            if (node->as.return_stmt.expr) {
                ast_print(node->as.return_stmt.expr, indent + 1);
            }
            break;

        case AST_EXPR_STMT:
            printf("ExprStmt\n");
            ast_print(node->as.expr_stmt.expr, indent + 1);
            break;

        case AST_BINARY_EXPR:
            printf("BinaryExpr: %s\n", token_type_name(node->as.binary_expr.op));
            ast_print(node->as.binary_expr.left, indent + 1);
            ast_print(node->as.binary_expr.right, indent + 1);
            break;

        case AST_UNARY_EXPR:
            printf("UnaryExpr: %s\n", token_type_name(node->as.unary_expr.op));
            ast_print(node->as.unary_expr.operand, indent + 1);
            break;

        case AST_LITERAL_INT:
            printf("IntLit: %d\n", node->as.lit_int.int_val);
            break;

        case AST_LITERAL_STRING:
            printf("StringLit: \"%s\"\n", node->as.lit_str.str_val);
            break;

        case AST_LITERAL_BOOL:
            printf("BoolLit: %s\n", node->as.lit_bool.bool_val ? "true" : "false");
            break;

        case AST_VAR_EXPR:
            printf("VarRef: %s\n", node->as.var_expr.name);
            break;

        case AST_FIELD_ACCESS_EXPR:
            printf("FieldAccess: .%s\n", node->as.field_access.field_name);
            ast_print(node->as.field_access.object, indent + 1);
            break;

        case AST_NEW_EXPR:
            printf("NewExpr: %s\n", node->as.new_expr.struct_name);
            break;

        case AST_CALL_EXPR:
            printf("CallExpr: %s()\n", node->as.call_expr.func_name);
            for (ASTNodeList *l = node->as.call_expr.args; l; l = l->next) {
                ast_print(l->node, indent + 1);
            }
            break;
    }
}
