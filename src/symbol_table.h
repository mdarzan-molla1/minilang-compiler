#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

typedef enum {
    SYM_VAR,
    SYM_FUNC,
    SYM_STRUCT
} SymbolKind;

typedef struct Symbol Symbol;

typedef struct FieldInfo {
    char name[64];
    char type_name[64];
    struct FieldInfo *next;
} FieldInfo;

typedef struct ParamInfo {
    char name[64];
    char type_name[64];
    struct ParamInfo *next;
} ParamInfo;

struct Symbol {
    char name[64];
    SymbolKind kind;
    char type_name[64]; // return type for funcs, struct type for vars
    int line;
    int col;

    // For functions
    ParamInfo *params;
    int param_count;

    // For structs
    FieldInfo *fields;

    struct Symbol *next; // in bucket/list
};

typedef struct Scope {
    struct Scope *parent; // Static scoping link
    Symbol *symbols;      // Linked list of symbols in this scope
    int scope_level;
} Scope;

Scope* scope_create(Scope *parent);
void scope_destroy(Scope *scope);

// Insert into current scope only (returns 0 if already exists)
int scope_insert(Scope *scope, Symbol *sym);

// Look up only in current scope
Symbol* scope_lookup_current(Scope *scope, const char *name);

// Look up traversing parent scopes (static lexical lookup)
Symbol* scope_lookup(Scope *scope, const char *name);

// Helper constructors
Symbol* symbol_new_var(const char *name, const char *type_name, int line, int col);
Symbol* symbol_new_func(const char *name, const char *ret_type, int line, int col);
Symbol* symbol_new_struct(const char *name, int line, int col);

#endif // SYMBOL_TABLE_H
