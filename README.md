# MiniLang Compiler

A hand-crafted compiler front-end and back-end for the **MiniLang** programming language.

**Student Roll Number:** `241030042`  
**Personalized Variant (Roll mod 4):** `241030042 % 4 = 2`  
**Variant Feature Set:** **Structs / Records**, plus **Nested Functions with Static Scoping**.  
**Keyword Mapping:** Documented in [KEYWORDS.md](KEYWORDS.md) (reversed standard keywords suffixed with `42`).

---

## 1. Architecture & Implemented Components

1. **DFA-based Hand-written Lexer (`src/lexer.c`, `src/lexer.h`, `src/tokens.h`)**:
   - Implements explicit deterministic finite automaton transitions for keywords, identifiers, integers, string literals, and compound operators (`==`, `!=`, `<=`, `>=`, `&&`, `||`).
   - Tracks line and column positions.
   - Reports exact line and column errors for unrecognized characters or unclosed literals.

2. **Recursive-Descent LL(1) Parser (`src/parser.c`, `src/parser.h`, `src/ast.c`, `src/ast.h`)**:
   - Fully hand-written (no Lex/Yacc, ANTLR, or Flex/Bison used).
   - Grammar is strictly left-factored and free of left-recursion (documented in [GRAMMAR.md](GRAMMAR.md)).
   - Builds a clean Abstract Syntax Tree (AST) supporting struct declarations, nested function declarations, and nested blocks.

3. **Symbol Table & Semantic Analysis (`src/symbol_table.c`, `src/semantic.c`)**:
   - Implements a scope tree with parent static links supporting arbitrary levels of nested lexical scopes.
   - Rigorous type checking and verification across arithmetic, logic, struct instantiation, and field accesses.
   - Detects and reports **5 distinct semantic errors**:
     - *Error 1:* Undeclared identifier
     - *Error 2:* Duplicate declaration in the same scope
     - *Error 3:* Type mismatch in assignment / initialization
     - *Error 4:* Accessing non-existent field on a struct
     - *Error 5:* Function return type mismatch

4. **Three-Address Code (TAC) Generation (`src/tac.c`, `src/tac.h`)**:
   - Lowers AST into linear TAC instructions using temporary variables (`t1`, `t2`, ...) and labels (`L1`, `L2`, ...).
   - Supports conditional jumps, function calls, field loads/stores, and struct heap allocation.

5. **TAC Optimizations (`src/optimizer.c`, `src/optimizer.h`)**:
   - **Optimization 1 - Constant Folding:** Evaluates constant arithmetic and comparison expressions at compile-time (e.g., `t1 = 10 + 20` becomes `t1 = 30`).
   - **Optimization 2 - Dead Code Elimination:** Eliminates unreachable instructions following unconditional jumps (`goto`, `return`) and unused temporary definitions.
   - Compares and displays TAC before and after optimization.

6. **Backend: Stack-Machine Code & Virtual Machine Runner (`src/vm.c`, `src/vm.h`)**:
   - Compiles TAC into stack-machine bytecode instructions (`PUSH_INT`, `STORE_VAR`, `LOAD_VAR`, `ADD`, `SUB`, `JUMP_IF_FALSE`, `CALL`, `RET`, `ALLOC_STRUCT`, `GET_FIELD`, `SET_FIELD`).
   - Includes a stack-based virtual machine interpreter that executes the bytecode with call frames, lexical scope lookup for nested functions, and dynamic struct records.

---

## 2. Building the Compiler

The compiler is written in standard portable C (C99/C11) with zero external dependencies.

### Using GCC / Clang:
```bash
gcc -O2 -o minilang src/*.c
```

### Using MSVC (Developer Command Prompt):
```cmd
cl /O2 /Fe:minilang.exe src\*.c
```

---

## 3. Running and Usage

Run any MiniLang program directly:
```bash
./minilang tests/test01_arithmetic.ml
```

### Command-line Flags
You can inspect intermediate compiler phases using flags:
* `--tokens`: Display the lexical token stream with line and column indices.
* `--ast`: Pretty-print the generated Abstract Syntax Tree (AST).
* `--tac`: Display Three-Address Code before and after optimizations, with optimization statistics.
* `--vm`: Disassemble the generated stack-machine bytecode.
* `--no-run`: Compile and verify without executing.

**Example full inspection:**
```bash
./minilang tests/test07_struct_basic.ml --tokens --ast --tac --vm
```

---

## 4. Test Suite (`tests/`)

The repository contains **15 test programs**:

### Valid Test Programs (10 tests)
* `test01_arithmetic.ml`: Arithmetic operations and precedence rules.
* `test02_variables.ml`: Variable declarations, updates, and block scoping.
* `test03_if_else.ml`: Conditional branching and relational operators.
* `test04_boolean_logic.ml`: Boolean literals (`eurt42`, `eslaf42`) and logical operators (`&&`, `||`, `!`).
* `test05_functions.ml`: Function declarations, parameter passing, and return values.
* `test06_nested_functions.ml`: **Nested functions with static scoping** (Variant 2).
* `test07_struct_basic.ml`: **Struct declaration, instantiation (`wen42`), and field access** (Variant 2).
* `test08_struct_mutation.ml`: Struct field modification and passing structs to functions.
* `test09_constant_folding.ml`: Verification of constant folding and dead code elimination passes.
* `test10_complex_integration.ml`: Full integration testing structs, nested functions, static scoping, and control flow.

### Invalid Test Programs (5 tests for Error Handling)
* `invalid01_undeclared_var.ml`: Tests detection of undeclared identifiers.
* `invalid02_duplicate_var.ml`: Tests detection of duplicate declarations in the same scope.
* `invalid03_type_mismatch.ml`: Tests detection of assignment type mismatch (string to int).
* `invalid04_unknown_field.ml`: Tests detection of non-existent struct field access.
* `invalid05_return_mismatch.ml`: Tests detection of function return type mismatch.

### Running all tests:
```powershell
Get-ChildItem tests\*.ml | ForEach-Object { Write-Host "`n=== $($_.Name) ==="; .\minilang $_.FullName }
```