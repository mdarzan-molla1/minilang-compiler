#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"

static Token current_tok;
static int had_error = 0;

int parser_has_error(void) {
    return had_error;
}

static void advance(void) {
    current_tok = lexer_next_token();
}

static int check(TokenType type) {
    return current_tok.type == type;
}

static int match(TokenType type) {
    if (check(type)) {
        advance();
        return 1;
    }
    return 0;
}

static void syntax_error(const char *msg) {
    had_error = 1;
    fprintf(stderr, "Syntax Error at Line %d, Col %d: %s (got '%s')\n",
            current_tok.line, current_tok.col, msg, current_tok.text);
}

static int consume(TokenType type, const char *msg) {
    if (check(type)) {
        advance();
        return 1;
    }
    syntax_error(msg);
    return 0;
}

// Forward declarations
static ASTNode* parse_statement(void);
static ASTNode* parse_expression(void);
static ASTNode* parse_block(void);
static ASTNode* parse_func_decl(void);
static ASTNode* parse_var_decl(void);

// Parse type specification: tni42, loob42, gnirts42, diov42, or struct identifier
static void parse_type_name(char *dest) {
    if (check(TOKEN_INT) || check(TOKEN_BOOL) || check(TOKEN_STRING) || 
        check(TOKEN_VOID) || check(TOKEN_IDENTIFIER)) {
        strcpy(dest, current_tok.text);
        advance();
    } else {
        syntax_error("Expected type name");
        strcpy(dest, "unknown");
    }
}

// Primary expression: literal, (expr), new Struct, id, id.field, id(args...)
static ASTNode* parse_primary(void) {
    int line = current_tok.line;
    int col = current_tok.col;

    // Integer literal
    if (check(TOKEN_INT_LIT)) {
        ASTNode *node = ast_new_node(AST_LITERAL_INT, line, col);
        node->as.lit_int.int_val = current_tok.int_val;
        strcpy(node->type_name, "tni42");
        advance();
        return node;
    }

    // String literal
    if (check(TOKEN_STRING_LIT)) {
        ASTNode *node = ast_new_node(AST_LITERAL_STRING, line, col);
        strncpy(node->as.lit_str.str_val, current_tok.text, 127);
        strcpy(node->type_name, "gnirts42");
        advance();
        return node;
    }

    // Boolean literals
    if (check(TOKEN_TRUE) || check(TOKEN_FALSE)) {
        ASTNode *node = ast_new_node(AST_LITERAL_BOOL, line, col);
        node->as.lit_bool.bool_val = check(TOKEN_TRUE) ? 1 : 0;
        strcpy(node->type_name, "loob42");
        advance();
        return node;
    }

    // Instantiation: wen42 StructName
    if (match(TOKEN_NEW)) {
        if (!check(TOKEN_IDENTIFIER)) {
            syntax_error("Expected struct name after wen42");
            return NULL;
        }
        ASTNode *node = ast_new_node(AST_NEW_EXPR, line, col);
        strcpy(node->as.new_expr.struct_name, current_tok.text);
        strcpy(node->type_name, current_tok.text);
        advance();
        return node;
    }

    // Grouped expression: ( expr )
    if (match(TOKEN_LPAREN)) {
        ASTNode *expr = parse_expression();
        consume(TOKEN_RPAREN, "Expected ')' after expression");
        return expr;
    }

    // Identifier base (could be variable, function call, or struct field access)
    if (check(TOKEN_IDENTIFIER)) {
        char name[64];
        strcpy(name, current_tok.text);
        advance();

        ASTNode *curr = NULL;

        // Function call: name(args...)
        if (match(TOKEN_LPAREN)) {
            curr = ast_new_node(AST_CALL_EXPR, line, col);
            strcpy(curr->as.call_expr.func_name, name);
            curr->as.call_expr.args = NULL;

            if (!check(TOKEN_RPAREN)) {
                do {
                    ASTNode *arg = parse_expression();
                    curr->as.call_expr.args = ast_list_append(curr->as.call_expr.args, arg);
                } while (match(TOKEN_COMMA));
            }
            consume(TOKEN_RPAREN, "Expected ')' after arguments");
        } else {
            curr = ast_new_node(AST_VAR_EXPR, line, col);
            strcpy(curr->as.var_expr.name, name);
        }

        // Postfix field access: .field
        while (match(TOKEN_DOT)) {
            if (!check(TOKEN_IDENTIFIER)) {
                syntax_error("Expected field name after '.'");
                break;
            }
            ASTNode *field_node = ast_new_node(AST_FIELD_ACCESS_EXPR, current_tok.line, current_tok.col);
            field_node->as.field_access.object = curr;
            strcpy(field_node->as.field_access.field_name, current_tok.text);
            advance();
            curr = field_node;
        }

        return curr;
    }

    syntax_error("Unexpected token in expression");
    advance();
    return NULL;
}

// Unary expression: -expr, !expr
static ASTNode* parse_unary(void) {
    if (check(TOKEN_MINUS) || check(TOKEN_NOT)) {
        int line = current_tok.line;
        int col = current_tok.col;
        TokenType op = current_tok.type;
        advance();
        ASTNode *operand = parse_unary();
        ASTNode *node = ast_new_node(AST_UNARY_EXPR, line, col);
        node->as.unary_expr.op = op;
        node->as.unary_expr.operand = operand;
        return node;
    }
    return parse_primary();
}

// Multiplicative: * /
static ASTNode* parse_multiplicative(void) {
    ASTNode *left = parse_unary();
    while (check(TOKEN_STAR) || check(TOKEN_SLASH)) {
        int line = current_tok.line;
        int col = current_tok.col;
        TokenType op = current_tok.type;
        advance();
        ASTNode *right = parse_unary();
        ASTNode *node = ast_new_node(AST_BINARY_EXPR, line, col);
        node->as.binary_expr.op = op;
        node->as.binary_expr.left = left;
        node->as.binary_expr.right = right;
        left = node;
    }
    return left;
}

// Additive: + -
static ASTNode* parse_additive(void) {
    ASTNode *left = parse_multiplicative();
    while (check(TOKEN_PLUS) || check(TOKEN_MINUS)) {
        int line = current_tok.line;
        int col = current_tok.col;
        TokenType op = current_tok.type;
        advance();
        ASTNode *right = parse_multiplicative();
        ASTNode *node = ast_new_node(AST_BINARY_EXPR, line, col);
        node->as.binary_expr.op = op;
        node->as.binary_expr.left = left;
        node->as.binary_expr.right = right;
        left = node;
    }
    return left;
}

// Relational: < <= > >=
static ASTNode* parse_relational(void) {
    ASTNode *left = parse_additive();
    while (check(TOKEN_LT) || check(TOKEN_LTE) || check(TOKEN_GT) || check(TOKEN_GTE)) {
        int line = current_tok.line;
        int col = current_tok.col;
        TokenType op = current_tok.type;
        advance();
        ASTNode *right = parse_additive();
        ASTNode *node = ast_new_node(AST_BINARY_EXPR, line, col);
        node->as.binary_expr.op = op;
        node->as.binary_expr.left = left;
        node->as.binary_expr.right = right;
        left = node;
    }
    return left;
}

// Equality: == !=
static ASTNode* parse_equality(void) {
    ASTNode *left = parse_relational();
    while (check(TOKEN_EQ) || check(TOKEN_NEQ)) {
        int line = current_tok.line;
        int col = current_tok.col;
        TokenType op = current_tok.type;
        advance();
        ASTNode *right = parse_relational();
        ASTNode *node = ast_new_node(AST_BINARY_EXPR, line, col);
        node->as.binary_expr.op = op;
        node->as.binary_expr.left = left;
        node->as.binary_expr.right = right;
        left = node;
    }
    return left;
}

// Logical AND: &&
static ASTNode* parse_logical_and(void) {
    ASTNode *left = parse_equality();
    while (check(TOKEN_AND)) {
        int line = current_tok.line;
        int col = current_tok.col;
        TokenType op = current_tok.type;
        advance();
        ASTNode *right = parse_equality();
        ASTNode *node = ast_new_node(AST_BINARY_EXPR, line, col);
        node->as.binary_expr.op = op;
        node->as.binary_expr.left = left;
        node->as.binary_expr.right = right;
        left = node;
    }
    return left;
}

// Logical OR: ||
static ASTNode* parse_expression(void) {
    ASTNode *left = parse_logical_and();
    while (check(TOKEN_OR)) {
        int line = current_tok.line;
        int col = current_tok.col;
        TokenType op = current_tok.type;
        advance();
        ASTNode *right = parse_logical_and();
        ASTNode *node = ast_new_node(AST_BINARY_EXPR, line, col);
        node->as.binary_expr.op = op;
        node->as.binary_expr.left = left;
        node->as.binary_expr.right = right;
        left = node;
    }
    return left;
}

// Variable Declaration: tel42 type name [= expr] ;
static ASTNode* parse_var_decl(void) {
    int line = current_tok.line;
    int col = current_tok.col;
    consume(TOKEN_LET, "Expected tel42");

    char type_name[64];
    parse_type_name(type_name);

    if (!check(TOKEN_IDENTIFIER)) {
        syntax_error("Expected variable name");
        return NULL;
    }
    char var_name[64];
    strcpy(var_name, current_tok.text);
    advance();

    ASTNode *init = NULL;
    if (match(TOKEN_ASSIGN)) {
        init = parse_expression();
    }
    consume(TOKEN_SEMICOLON, "Expected ';' after variable declaration");

    ASTNode *node = ast_new_node(AST_VAR_DECL, line, col);
    strcpy(node->type_name, type_name);
    strcpy(node->as.var_decl.name, var_name);
    node->as.var_decl.init_expr = init;
    return node;
}

// Struct Declaration: tcurts42 Name { tel42 type field; ... }
static ASTNode* parse_struct_decl(void) {
    int line = current_tok.line;
    int col = current_tok.col;
    consume(TOKEN_STRUCT, "Expected tcurts42");

    if (!check(TOKEN_IDENTIFIER)) {
        syntax_error("Expected struct name");
        return NULL;
    }
    ASTNode *node = ast_new_node(AST_STRUCT_DECL, line, col);
    strcpy(node->as.struct_decl.name, current_tok.text);
    advance();

    consume(TOKEN_LBRACE, "Expected '{' before struct body");
    node->as.struct_decl.fields = NULL;

    while (!check(TOKEN_RBRACE) && !check(TOKEN_EOF)) {
        if (check(TOKEN_LET)) {
            ASTNode *field = parse_var_decl();
            node->as.struct_decl.fields = ast_list_append(node->as.struct_decl.fields, field);
        } else {
            syntax_error("Expected field declaration with tel42");
            advance();
        }
    }
    consume(TOKEN_RBRACE, "Expected '}' after struct body");
    return node;
}

// Function Declaration: noitcnuf42 name(params) : return_type { nested_funcs... stmts... }
static ASTNode* parse_func_decl(void) {
    int line = current_tok.line;
    int col = current_tok.col;
    consume(TOKEN_FUNCTION, "Expected noitcnuf42");

    if (!check(TOKEN_IDENTIFIER)) {
        syntax_error("Expected function name");
        return NULL;
    }
    ASTNode *node = ast_new_node(AST_FUNC_DECL, line, col);
    strcpy(node->as.func_decl.name, current_tok.text);
    advance();

    consume(TOKEN_LPAREN, "Expected '(' after function name");
    node->as.func_decl.params = NULL;

    if (!check(TOKEN_RPAREN)) {
        do {
            int p_line = current_tok.line;
            int p_col = current_tok.col;
            char p_type[64];
            parse_type_name(p_type);

            if (!check(TOKEN_IDENTIFIER)) {
                syntax_error("Expected parameter name");
                break;
            }
            ASTNode *param = ast_new_node(AST_VAR_DECL, p_line, p_col);
            strcpy(param->type_name, p_type);
            strcpy(param->as.var_decl.name, current_tok.text);
            advance();

            node->as.func_decl.params = ast_list_append(node->as.func_decl.params, param);
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RPAREN, "Expected ')' after parameters");

    consume(TOKEN_COLON, "Expected ':' before return type");
    parse_type_name(node->type_name);

    consume(TOKEN_LBRACE, "Expected '{' to start function body");

    // Inside body: parse all nested functions and statements in linear order
    ASTNode *body_block = ast_new_node(AST_BLOCK, line, col);
    body_block->as.block.statements = NULL;

    while (!check(TOKEN_RBRACE) && !check(TOKEN_EOF)) {
        if (check(TOKEN_FUNCTION)) {
            ASTNode *nested = parse_func_decl();
            body_block->as.block.statements = ast_list_append(body_block->as.block.statements, nested);
        } else {
            ASTNode *stmt = parse_statement();
            if (stmt) {
                body_block->as.block.statements = ast_list_append(body_block->as.block.statements, stmt);
            }
        }
    }
    consume(TOKEN_RBRACE, "Expected '}' to close function body");
    node->as.func_decl.body = body_block;
    node->as.func_decl.nested_funcs = NULL;
    return node;
}

// If Statement: fi42 (condition) statement [esle42 statement]
static ASTNode* parse_if_stmt(void) {
    int line = current_tok.line;
    int col = current_tok.col;
    consume(TOKEN_IF, "Expected fi42");

    consume(TOKEN_LPAREN, "Expected '(' after fi42");
    ASTNode *cond = parse_expression();
    consume(TOKEN_RPAREN, "Expected ')' after condition");

    ASTNode *then_branch = parse_statement();
    ASTNode *else_branch = NULL;

    if (match(TOKEN_ELSE)) {
        else_branch = parse_statement();
    }

    ASTNode *node = ast_new_node(AST_IF_STMT, line, col);
    node->as.if_stmt.condition = cond;
    node->as.if_stmt.then_branch = then_branch;
    node->as.if_stmt.else_branch = else_branch;
    return node;
}

// Print Statement: tnirp42(expr);
static ASTNode* parse_print_stmt(void) {
    int line = current_tok.line;
    int col = current_tok.col;
    consume(TOKEN_PRINT, "Expected tnirp42");

    consume(TOKEN_LPAREN, "Expected '(' after tnirp42");
    ASTNode *expr = parse_expression();
    consume(TOKEN_RPAREN, "Expected ')' after print argument");
    consume(TOKEN_SEMICOLON, "Expected ';' after print statement");

    ASTNode *node = ast_new_node(AST_PRINT_STMT, line, col);
    node->as.print_stmt.expr = expr;
    return node;
}

// Return Statement: nruter42 [expr];
static ASTNode* parse_return_stmt(void) {
    int line = current_tok.line;
    int col = current_tok.col;
    consume(TOKEN_RETURN, "Expected nruter42");

    ASTNode *expr = NULL;
    if (!check(TOKEN_SEMICOLON)) {
        expr = parse_expression();
    }
    consume(TOKEN_SEMICOLON, "Expected ';' after return");

    ASTNode *node = ast_new_node(AST_RETURN_STMT, line, col);
    node->as.return_stmt.expr = expr;
    return node;
}

// Block: { statements... }
static ASTNode* parse_block(void) {
    int line = current_tok.line;
    int col = current_tok.col;
    consume(TOKEN_LBRACE, "Expected '{'");

    ASTNode *block = ast_new_node(AST_BLOCK, line, col);
    block->as.block.statements = NULL;

    while (!check(TOKEN_RBRACE) && !check(TOKEN_EOF)) {
        ASTNode *stmt = parse_statement();
        if (stmt) {
            block->as.block.statements = ast_list_append(block->as.block.statements, stmt);
        }
    }
    consume(TOKEN_RBRACE, "Expected '}'");
    return block;
}

// Statement dispatcher
static ASTNode* parse_statement(void) {
    if (check(TOKEN_LET)) {
        return parse_var_decl();
    }
    if (check(TOKEN_IF)) {
        return parse_if_stmt();
    }
    if (check(TOKEN_PRINT)) {
        return parse_print_stmt();
    }
    if (check(TOKEN_RETURN)) {
        return parse_return_stmt();
    }
    if (check(TOKEN_LBRACE)) {
        return parse_block();
    }

    // Expression statement or assignment
    int line = current_tok.line;
    int col = current_tok.col;
    ASTNode *expr = parse_expression();

    // Check if this is an assignment statement: target = value;
    if (match(TOKEN_ASSIGN)) {
        ASTNode *val = parse_expression();
        consume(TOKEN_SEMICOLON, "Expected ';' after assignment");
        ASTNode *assign = ast_new_node(AST_ASSIGN_STMT, line, col);
        assign->as.assign_stmt.target = expr;
        assign->as.assign_stmt.value = val;
        return assign;
    }

    consume(TOKEN_SEMICOLON, "Expected ';' after expression statement");
    ASTNode *stmt = ast_new_node(AST_EXPR_STMT, line, col);
    stmt->as.expr_stmt.expr = expr;
    return stmt;
}

// Top level: Structs, Functions, and Global Variables
ASTNode* parser_parse(void) {
    had_error = 0;
    advance(); // load first token

    ASTNode *prog = ast_new_node(AST_PROGRAM, 1, 1);
    prog->as.program.statements = NULL;

    while (!check(TOKEN_EOF)) {
        ASTNode *item = NULL;
        if (check(TOKEN_STRUCT)) {
            item = parse_struct_decl();
        } else if (check(TOKEN_FUNCTION)) {
            item = parse_func_decl();
        } else if (check(TOKEN_LET)) {
            item = parse_var_decl();
        } else {
            syntax_error("Unexpected top-level declaration");
            advance();
        }

        if (item) {
            prog->as.program.statements = ast_list_append(prog->as.program.statements, item);
        }
    }

    return prog;
}
