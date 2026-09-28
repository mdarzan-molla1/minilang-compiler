#ifndef AST_H
#define AST_H

#include "tokens.h"

typedef enum {
    AST_PROGRAM,
    AST_STRUCT_DECL,
    AST_FUNC_DECL,
    AST_VAR_DECL,
    AST_BLOCK,
    AST_IF_STMT,
    AST_ASSIGN_STMT,
    AST_PRINT_STMT,
    AST_RETURN_STMT,
    AST_EXPR_STMT,

    // Expressions
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_LITERAL_INT,
    AST_LITERAL_STRING,
    AST_LITERAL_BOOL,
    AST_VAR_EXPR,
    AST_FIELD_ACCESS_EXPR,
    AST_NEW_EXPR,
    AST_CALL_EXPR
} ASTNodeType;

typedef struct ASTNode ASTNode;

// Linked list of AST nodes (for statements, parameters, fields, arguments)
typedef struct ASTNodeList {
    ASTNode *node;
    struct ASTNodeList *next;
} ASTNodeList;

struct ASTNode {
    ASTNodeType type;
    int line;
    int col;

    // Type name for declarations / return types (e.g. "tni42", "Point", "diov42")
    char type_name[64];

    // Node-specific data
    union {
        // Program or Block: list of declarations / statements
        struct {
            ASTNodeList *statements;
        } program, block;

        // Struct declaration: struct Name { fields... }
        struct {
            char name[64];
            ASTNodeList *fields; // list of VarDecl
        } struct_decl;

        // Function declaration (supports nested functions)
        struct {
            char name[64];
            ASTNodeList *params;       // list of VarDecl
            ASTNodeList *nested_funcs; // nested function declarations
            ASTNode *body;             // block
        } func_decl;

        // Variable declaration: tel42 type name [= init];
        struct {
            char name[64];
            ASTNode *init_expr;
        } var_decl;

        // If statement: fi42 (cond) then_branch esle42 else_branch
        struct {
            ASTNode *condition;
            ASTNode *then_branch;
            ASTNode *else_branch;
        } if_stmt;

        // Assignment: target = value
        // target can be VarExpr or FieldAccessExpr
        struct {
            ASTNode *target;
            ASTNode *value;
        } assign_stmt;

        // Print statement: tnirp42(expr);
        struct {
            ASTNode *expr;
        } print_stmt;

        // Return statement: nruter42 [expr];
        struct {
            ASTNode *expr;
        } return_stmt;

        // Expression statement: expr;
        struct {
            ASTNode *expr;
        } expr_stmt;

        // Binary expression: left op right
        struct {
            TokenType op;
            ASTNode *left;
            ASTNode *right;
        } binary_expr;

        // Unary expression: op operand
        struct {
            TokenType op;
            ASTNode *operand;
        } unary_expr;

        // Literals
        struct {
            int int_val;
        } lit_int;

        struct {
            char str_val[128];
        } lit_str;

        struct {
            int bool_val;
        } lit_bool;

        // Variable reference
        struct {
            char name[64];
        } var_expr;

        // Field access: object.field
        struct {
            ASTNode *object;
            char field_name[64];
        } field_access;

        // Object instantiation: wen42 StructName
        struct {
            char struct_name[64];
        } new_expr;

        // Function call: func(args...)
        struct {
            char func_name[64];
            ASTNodeList *args;
        } call_expr;
    } as;
};

// Node constructor functions
ASTNode* ast_new_node(ASTNodeType type, int line, int col);
ASTNodeList* ast_list_append(ASTNodeList *list, ASTNode *node);
void ast_print(ASTNode *node, int indent);
void ast_free(ASTNode *node);

#endif // AST_H
