#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokens.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "semantic.h"
#include "tac.h"
#include "optimizer.h"
#include "vm.h"

static char* read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open file '%s'\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = (char*)malloc(length + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }
    size_t read = fread(buffer, 1, length, f);
    buffer[read] = '\0';
    fclose(f);
    return buffer;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("MiniLang Compiler (Roll 241030042 - Variant 2: Structs & Nested Functions)\n");
        printf("Usage: %s <source_file> [--tokens] [--ast] [--tac] [--vm] [--no-run]\n", argv[0]);
        return 1;
    }

    const char *filepath = argv[1];
    int flag_tokens = 0;
    int flag_ast = 0;
    int flag_tac = 0;
    int flag_vm = 0;
    int flag_run = 1;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0) flag_tokens = 1;
        else if (strcmp(argv[i], "--ast") == 0) flag_ast = 1;
        else if (strcmp(argv[i], "--tac") == 0) flag_tac = 1;
        else if (strcmp(argv[i], "--vm") == 0) flag_vm = 1;
        else if (strcmp(argv[i], "--no-run") == 0) flag_run = 0;
    }

    char *source = read_file(filepath);
    if (!source) return 1;

    // Optional: Print tokens
    if (flag_tokens) {
        printf("\n================ [1. LEXICAL ANALYSIS] ================\n");
        lexer_init(source);
        while (1) {
            Token tok = lexer_next_token();
            printf("Line %-3d Col %-3d | %-20s | %s\n", tok.line, tok.col, token_type_name(tok.type), tok.text);
            if (tok.type == TOKEN_EOF || tok.type == TOKEN_ERROR) break;
        }
    }

    // 1. Parsing
    lexer_init(source);
    ASTNode *root = parser_parse();
    if (parser_has_error()) {
        fprintf(stderr, "\n[Compilation Failed during Parsing]\n");
        free(source);
        return 1;
    }

    if (flag_ast) {
        printf("\n================ [2. ABSTRACT SYNTAX TREE] ================\n");
        ast_print(root, 0);
    }

    // 2. Semantic Analysis
    int sem_errors = semantic_analyze(root);
    if (sem_errors > 0) {
        fprintf(stderr, "\n[Compilation Failed: %d Semantic Error(s) Detected]\n", sem_errors);
        free(source);
        return 1;
    }

    // 3. Three-Address Code (TAC) Generation
    TACProgram *tac_prog = tac_generate(root);

    if (flag_tac) {
        printf("\n================ [3. TAC (BEFORE OPTIMIZATION)] ================\n");
        tac_print(tac_prog);
    }

    // 4. Optimization (Constant Folding & Dead Code Elimination)
    if (flag_tac) {
        printf("\n================ [4. RUNNING OPTIMIZATIONS] ================\n");
    }
    optimize_tac(tac_prog);

    if (flag_tac) {
        printf("\n================ [5. TAC (AFTER OPTIMIZATION)] ================\n");
        tac_print(tac_prog);
    }

    // 5. Backend: Stack-Machine Code Generation
    BytecodeProgram *bytecode = vm_compile_tac(tac_prog);

    if (flag_vm) {
        vm_disassemble(bytecode);
    }

    // 6. Execution
    if (flag_run) {
        if (flag_tokens || flag_ast || flag_tac || flag_vm) {
            printf("\n================ [6. EXECUTION OUTPUT] ================\n");
        }
        vm_execute(bytecode);
    }

    vm_free(bytecode);
    tac_free(tac_prog);
    free(source);

    return 0;
}
