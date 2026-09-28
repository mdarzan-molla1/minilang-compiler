#ifndef TOKENS_H
#define TOKENS_H

// Token types for MiniLang (Variant 2: Roll 241030042)
// Keywords derived by reversing word + "42"
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_ERROR,

    // Keywords
    TOKEN_STRUCT,       // tcurts42
    TOKEN_FUNCTION,     // noitcnuf42
    TOKEN_RETURN,       // nruter42
    TOKEN_IF,           // fi42
    TOKEN_ELSE,         // esle42
    TOKEN_LET,          // tel42
    TOKEN_INT,          // tni42
    TOKEN_BOOL,         // loob42
    TOKEN_STRING,       // gnirts42
    TOKEN_VOID,         // diov42
    TOKEN_TRUE,         // eurt42
    TOKEN_FALSE,        // eslaf42
    TOKEN_PRINT,        // tnirp42
    TOKEN_NEW,          // wen42

    // Identifiers and Literals
    TOKEN_IDENTIFIER,
    TOKEN_INT_LIT,
    TOKEN_STRING_LIT,

    // Operators
    TOKEN_ASSIGN,       // =
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_STAR,         // *
    TOKEN_SLASH,        // /
    TOKEN_EQ,           // ==
    TOKEN_NEQ,          // !=
    TOKEN_LT,           // <
    TOKEN_LTE,          // <=
    TOKEN_GT,           // >
    TOKEN_GTE,          // >=
    TOKEN_AND,          // &&
    TOKEN_OR,           // ||
    TOKEN_NOT,          // !

    // Delimiters
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_SEMICOLON,    // ;
    TOKEN_COMMA,        // ,
    TOKEN_DOT,          // .
    TOKEN_COLON         // :
} TokenType;

typedef struct {
    TokenType type;
    char text[128];
    int int_val;
    int line;
    int col;
} Token;

const char* token_type_name(TokenType type);

#endif // TOKENS_H
