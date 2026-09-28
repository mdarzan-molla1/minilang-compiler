#ifndef LEXER_H
#define LEXER_H

#include "tokens.h"

// Initialize the lexer with source code string
void lexer_init(const char *source);

// Get the next token from source (DFA based)
Token lexer_next_token(void);

// Peek the next token without consuming it
Token lexer_peek_token(void);

// Check if a word matches a keyword
int is_keyword(const char *str, TokenType *type);

#endif // LEXER_H
