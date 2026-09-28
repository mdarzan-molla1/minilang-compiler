#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"
#include "ast.h"

// Parse the full token stream into an AST Program node
ASTNode* parser_parse(void);

// Error flag
int parser_has_error(void);

#endif // PARSER_H
