#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"

// Run complete semantic analysis on the AST
// Returns 0 on success, or number of semantic errors found
int semantic_analyze(ASTNode *root);

// Returns total semantic error count
int semantic_error_count(void);

#endif // SEMANTIC_H
