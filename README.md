# MiniLang Compiler

**Compiler Design Lab Project**  
**Author:** Md Arzan Molla  
**Roll:** 241030042  
**Assigned Variant:** Variant 2 (Roll % 4 = 2: Structs/records and nested functions with static scoping)  
**Keyword List:** Defined in [KEYWORDS.md](KEYWORDS.md) (standard keywords reversed with suffix `42`)  
**Grammar Document:** Detailed in [GRAMMAR.md](GRAMMAR.md)  
**Lab Report:** [REPORT.md](REPORT.md) / [REPORT.pdf](REPORT.pdf)  

---

## About This Project
This is my compiler design lab assignment. It is a working hand-written compiler front-end and back-end for a small language called **MiniLang**. No generator tools (like Lex, Yacc, Flex, Bison, or ANTLR) were used; every stage is written from scratch in C.

### What is implemented:
1. **Lexer (`src/lexer.c`):** Hand-written DFA that reads characters, tracks line and column numbers, and prints error coordinates if an unknown character or broken string is found.
2. **Parser (`src/parser.c`):** Recursive-descent parser that builds an Abstract Syntax Tree (AST). All left-recursion is removed and left-factoring is applied so it works top-down without backtracking.
3. **Symbol Table & Scopes (`src/symbol_table.c`, `src/semantic.c`):** Symbol table with parent scope links to handle static scoping for nested functions. It checks types and reports 5 distinct semantic errors.
4. **Three-Address Code (`src/tac.c`):** Translates the AST into linear Three-Address Code instructions with temporary variables.
5. **Optimizations (`src/optimizer.c`):** Runs Constant Folding (calculating constant math at compile time) and Dead Code Elimination (deleting dead code after returns).
6. **Backend Runner (`src/vm.c`):** Converts TAC into stack-machine bytecode and executes it using an internal virtual machine runner with support for struct records.

---

## How to Compile

The project is written in standard C99 and has no external library dependencies.

### On Linux or Windows (GCC/MinGW):
```bash
gcc -o minilang src/*.c
```

### On Windows (MSVC):
```cmd
cl /Fe:minilang.exe src\*.c
```

---

## How to Run

To run a test program directly:
```bash
./minilang tests/test01_arithmetic.ml
```

### Optional Debug Flags
You can pass flags to view each stage of the compiler:
- `./minilang tests/test01_arithmetic.ml --tokens` : Prints the list of scanned tokens.
- `./minilang tests/test01_arithmetic.ml --ast` : Prints the AST tree structure.
- `./minilang tests/test01_arithmetic.ml --tac` : Prints TAC before and after optimizations.
- `./minilang tests/test01_arithmetic.ml --vm` : Prints the stack-machine bytecode.

---

## Test Suite (`tests/`)

The repository includes 15 test files:

### 10 Valid Programs:
- `test01_arithmetic.ml` - Math operations and operator precedence.
- `test02_variables.ml` - Variable declaration, reassignment, and blocks.
- `test03_if_else.ml` - If and else condition checks.
- `test04_boolean_logic.ml` - Booleans and logical operators (`&&`, `||`, `!`).
- `test05_functions.ml` - Functions, arguments, and return values.
- `test06_nested_functions.ml` - Nested functions accessing outer variables (Variant 2).
- `test07_struct_basic.ml` - Struct definition, `wen42` instantiation, and field access (Variant 2).
- `test08_struct_mutation.ml` - Modifying struct fields and passing structs to functions.
- `test09_constant_folding.ml` - Tests compile-time math folding and dead code cleanup.
- `test10_complex_integration.ml` - Integration test combining structs, nested functions, and branching.

### 5 Error Programs (testing semantic checks):
- `invalid01_undeclared_var.ml` - Catches undeclared variable errors.
- `invalid02_duplicate_var.ml` - Catches duplicate declarations in the same scope.
- `invalid03_type_mismatch.ml` - Catches assigning a string to an integer variable.
- `invalid04_unknown_field.ml` - Catches accessing fields that don't exist in a struct.
- `invalid05_return_mismatch.ml` - Catches returning the wrong type from a function.