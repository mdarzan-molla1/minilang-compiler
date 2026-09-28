#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static const char *src = NULL;
static int pos = 0;
static int line = 1;
static int col = 1;

static Token peeked_token;
static int has_peeked = 0;

const char* token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        case TOKEN_STRUCT: return "KEYWORD(tcurts42)";
        case TOKEN_FUNCTION: return "KEYWORD(noitcnuf42)";
        case TOKEN_RETURN: return "KEYWORD(nruter42)";
        case TOKEN_IF: return "KEYWORD(fi42)";
        case TOKEN_ELSE: return "KEYWORD(esle42)";
        case TOKEN_LET: return "KEYWORD(tel42)";
        case TOKEN_INT: return "KEYWORD(tni42)";
        case TOKEN_BOOL: return "KEYWORD(loob42)";
        case TOKEN_STRING: return "KEYWORD(gnirts42)";
        case TOKEN_VOID: return "KEYWORD(diov42)";
        case TOKEN_TRUE: return "KEYWORD(eurt42)";
        case TOKEN_FALSE: return "KEYWORD(eslaf42)";
        case TOKEN_PRINT: return "KEYWORD(tnirp42)";
        case TOKEN_NEW: return "KEYWORD(wen42)";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INT_LIT: return "INT_LIT";
        case TOKEN_STRING_LIT: return "STRING_LIT";
        case TOKEN_ASSIGN: return "=";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_STAR: return "*";
        case TOKEN_SLASH: return "/";
        case TOKEN_EQ: return "==";
        case TOKEN_NEQ: return "!=";
        case TOKEN_LT: return "<";
        case TOKEN_LTE: return "<=";
        case TOKEN_GT: return ">";
        case TOKEN_GTE: return ">=";
        case TOKEN_AND: return "&&";
        case TOKEN_OR: return "||";
        case TOKEN_NOT: return "!";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_LBRACE: return "{";
        case TOKEN_RBRACE: return "}";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_COMMA: return ",";
        case TOKEN_DOT: return ".";
        case TOKEN_COLON: return ":";
        default: return "UNKNOWN";
    }
}

int is_keyword(const char *str, TokenType *type) {
    if (strcmp("tcurts42", str) == 0)   { *type = TOKEN_STRUCT; return 1; }
    if (strcmp("noitcnuf42", str) == 0) { *type = TOKEN_FUNCTION; return 1; }
    if (strcmp("nruter42", str) == 0)   { *type = TOKEN_RETURN; return 1; }
    if (strcmp("fi42", str) == 0)       { *type = TOKEN_IF; return 1; }
    if (strcmp("esle42", str) == 0)     { *type = TOKEN_ELSE; return 1; }
    if (strcmp("tel42", str) == 0)      { *type = TOKEN_LET; return 1; }
    if (strcmp("tni42", str) == 0)      { *type = TOKEN_INT; return 1; }
    if (strcmp("loob42", str) == 0)     { *type = TOKEN_BOOL; return 1; }
    if (strcmp("gnirts42", str) == 0)   { *type = TOKEN_STRING; return 1; }
    if (strcmp("diov42", str) == 0)     { *type = TOKEN_VOID; return 1; }
    if (strcmp("eurt42", str) == 0)     { *type = TOKEN_TRUE; return 1; }
    if (strcmp("eslaf42", str) == 0)    { *type = TOKEN_FALSE; return 1; }
    if (strcmp("tnirp42", str) == 0)    { *type = TOKEN_PRINT; return 1; }
    if (strcmp("wen42", str) == 0)      { *type = TOKEN_NEW; return 1; }
    return 0;
}

void lexer_init(const char *source) {
    src = source;
    pos = 0;
    line = 1;
    col = 1;
    has_peeked = 0;
}

static char peek_char(void) {
    if (!src || src[pos] == '\0') return '\0';
    return src[pos];
}

static char advance_char(void) {
    if (!src || src[pos] == '\0') return '\0';
    char c = src[pos++];
    if (c == '\n') {
        line++;
        col = 1;
    } else {
        col++;
    }
    return c;
}

// DFA-based next token scanner
static Token scan_token(void) {
    Token tok;
    memset(&tok, 0, sizeof(Token));

    while (1) {
        char c = peek_char();

        // End of input
        if (c == '\0') {
            tok.type = TOKEN_EOF;
            tok.line = line;
            tok.col = col;
            strcpy(tok.text, "EOF");
            return tok;
        }

        // Whitespace skipping
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance_char();
            continue;
        }

        // Skip line comments //
        if (c == '/' && src[pos + 1] == '/') {
            advance_char();
            advance_char();
            while (peek_char() != '\n' && peek_char() != '\0') {
                advance_char();
            }
            continue;
        }

        // Identifier or Keyword: [a-zA-Z_][a-zA-Z0-9_]*
        if (isalpha(c) || c == '_') {
            tok.line = line;
            tok.col = col;
            int i = 0;
            while (isalnum(peek_char()) || peek_char() == '_') {
                if (i < 127) {
                    tok.text[i++] = advance_char();
                } else {
                    advance_char();
                }
            }
            tok.text[i] = '\0';

            TokenType kw_type;
            if (is_keyword(tok.text, &kw_type)) {
                tok.type = kw_type;
            } else {
                tok.type = TOKEN_IDENTIFIER;
            }
            return tok;
        }

        // Integer literal: [0-9]+
        if (isdigit(c)) {
            tok.line = line;
            tok.col = col;
            int i = 0;
            while (isdigit(peek_char())) {
                if (i < 127) {
                    tok.text[i++] = advance_char();
                } else {
                    advance_char();
                }
            }
            tok.text[i] = '\0';
            tok.type = TOKEN_INT_LIT;
            tok.int_val = atoi(tok.text);
            return tok;
        }

        // String literal: "..."
        if (c == '"') {
            tok.line = line;
            tok.col = col;
            advance_char(); // consume opening "
            int i = 0;
            while (peek_char() != '"' && peek_char() != '\0' && peek_char() != '\n') {
                if (i < 127) {
                    tok.text[i++] = advance_char();
                } else {
                    advance_char();
                }
            }
            if (peek_char() == '"') {
                advance_char(); // consume closing "
                tok.text[i] = '\0';
                tok.type = TOKEN_STRING_LIT;
            } else {
                tok.type = TOKEN_ERROR;
                fprintf(stderr, "Lexer Error at Line %d, Col %d: Unterminated string literal\n", tok.line, tok.col);
            }
            return tok;
        }

        // Operators & delimiters (DFA transition for multi-char operators)
        tok.line = line;
        tok.col = col;
        char first = advance_char();

        switch (first) {
            case '=':
                if (peek_char() == '=') {
                    advance_char();
                    tok.type = TOKEN_EQ;
                    strcpy(tok.text, "==");
                } else {
                    tok.type = TOKEN_ASSIGN;
                    strcpy(tok.text, "=");
                }
                return tok;

            case '!':
                if (peek_char() == '=') {
                    advance_char();
                    tok.type = TOKEN_NEQ;
                    strcpy(tok.text, "!=");
                } else {
                    tok.type = TOKEN_NOT;
                    strcpy(tok.text, "!");
                }
                return tok;

            case '<':
                if (peek_char() == '=') {
                    advance_char();
                    tok.type = TOKEN_LTE;
                    strcpy(tok.text, "<=");
                } else {
                    tok.type = TOKEN_LT;
                    strcpy(tok.text, "<");
                }
                return tok;

            case '>':
                if (peek_char() == '=') {
                    advance_char();
                    tok.type = TOKEN_GTE;
                    strcpy(tok.text, ">=");
                } else {
                    tok.type = TOKEN_GT;
                    strcpy(tok.text, ">");
                }
                return tok;

            case '&':
                if (peek_char() == '&') {
                    advance_char();
                    tok.type = TOKEN_AND;
                    strcpy(tok.text, "&&");
                    return tok;
                } else {
                    tok.type = TOKEN_ERROR;
                    fprintf(stderr, "Lexer Error at Line %d, Col %d: Unexpected character '&'\n", tok.line, tok.col);
                    return tok;
                }

            case '|':
                if (peek_char() == '|') {
                    advance_char();
                    tok.type = TOKEN_OR;
                    strcpy(tok.text, "||");
                    return tok;
                } else {
                    tok.type = TOKEN_ERROR;
                    fprintf(stderr, "Lexer Error at Line %d, Col %d: Unexpected character '|'\n", tok.line, tok.col);
                    return tok;
                }

            case '+': tok.type = TOKEN_PLUS; strcpy(tok.text, "+"); return tok;
            case '-': tok.type = TOKEN_MINUS; strcpy(tok.text, "-"); return tok;
            case '*': tok.type = TOKEN_STAR; strcpy(tok.text, "*"); return tok;
            case '/': tok.type = TOKEN_SLASH; strcpy(tok.text, "/"); return tok;
            case '(': tok.type = TOKEN_LPAREN; strcpy(tok.text, "("); return tok;
            case ')': tok.type = TOKEN_RPAREN; strcpy(tok.text, ")"); return tok;
            case '{': tok.type = TOKEN_LBRACE; strcpy(tok.text, "{"); return tok;
            case '}': tok.type = TOKEN_RBRACE; strcpy(tok.text, "}"); return tok;
            case ';': tok.type = TOKEN_SEMICOLON; strcpy(tok.text, ";"); return tok;
            case ',': tok.type = TOKEN_COMMA; strcpy(tok.text, ","); return tok;
            case '.': tok.type = TOKEN_DOT; strcpy(tok.text, "."); return tok;
            case ':': tok.type = TOKEN_COLON; strcpy(tok.text, ":"); return tok;

            default:
                tok.type = TOKEN_ERROR;
                tok.text[0] = first;
                tok.text[1] = '\0';
                fprintf(stderr, "Lexer Error at Line %d, Col %d: Unrecognized character '%c'\n", tok.line, tok.col, first);
                return tok;
        }
    }
}

Token lexer_next_token(void) {
    if (has_peeked) {
        has_peeked = 0;
        return peeked_token;
    }
    return scan_token();
}

Token lexer_peek_token(void) {
    if (!has_peeked) {
        peeked_token = scan_token();
        has_peeked = 1;
    }
    return peeked_token;
}
