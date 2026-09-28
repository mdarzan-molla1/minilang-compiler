#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

Scope* scope_create(Scope *parent) {
    Scope *s = (Scope*)calloc(1, sizeof(Scope));
    s->parent = parent;
    s->scope_level = parent ? (parent->scope_level + 1) : 0;
    s->symbols = NULL;
    return s;
}

void scope_destroy(Scope *scope) {
    if (!scope) return;
    Symbol *curr = scope->symbols;
    while (curr) {
        Symbol *next = curr->next;
        // Free params / fields if any
        if (curr->kind == SYM_FUNC) {
            ParamInfo *p = curr->params;
            while (p) {
                ParamInfo *pn = p->next;
                free(p);
                p = pn;
            }
        } else if (curr->kind == SYM_STRUCT) {
            FieldInfo *f = curr->fields;
            while (f) {
                FieldInfo *fn = f->next;
                free(f);
                f = fn;
            }
        }
        free(curr);
        curr = next;
    }
    free(scope);
}

int scope_insert(Scope *scope, Symbol *sym) {
    if (!scope || !sym) return 0;

    // Check duplicate in current scope
    if (scope_lookup_current(scope, sym->name)) {
        return 0; // duplicate
    }

    sym->next = scope->symbols;
    scope->symbols = sym;
    return 1;
}

Symbol* scope_lookup_current(Scope *scope, const char *name) {
    if (!scope || !name) return NULL;
    for (Symbol *s = scope->symbols; s; s = s->next) {
        if (strcmp(s->name, name) == 0) {
            return s;
        }
    }
    return NULL;
}

Symbol* scope_lookup(Scope *scope, const char *name) {
    Scope *curr = scope;
    while (curr) {
        Symbol *sym = scope_lookup_current(curr, name);
        if (sym) return sym;
        curr = curr->parent; // traverse parent scope (static scoping)
    }
    return NULL;
}

Symbol* symbol_new_var(const char *name, const char *type_name, int line, int col) {
    Symbol *s = (Symbol*)calloc(1, sizeof(Symbol));
    strncpy(s->name, name, 63);
    s->kind = SYM_VAR;
    strncpy(s->type_name, type_name, 63);
    s->line = line;
    s->col = col;
    return s;
}

Symbol* symbol_new_func(const char *name, const char *ret_type, int line, int col) {
    Symbol *s = (Symbol*)calloc(1, sizeof(Symbol));
    strncpy(s->name, name, 63);
    s->kind = SYM_FUNC;
    strncpy(s->type_name, ret_type, 63);
    s->line = line;
    s->col = col;
    return s;
}

Symbol* symbol_new_struct(const char *name, int line, int col) {
    Symbol *s = (Symbol*)calloc(1, sizeof(Symbol));
    strncpy(s->name, name, 63);
    s->kind = SYM_STRUCT;
    strncpy(s->type_name, name, 63);
    s->line = line;
    s->col = col;
    return s;
}
