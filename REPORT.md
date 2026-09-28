# MiniLang Compiler Design Report

**Course:** Compiler Design Lab  
**Student Roll:** 241030042  
**Assigned Variant:** Variant 2 (`241030042 % 4 = 2` — Structs/records, plus nested functions with static scoping)  
**Submission Date:** September 2026  

---

## 1. Design Decisions

### 1.1 Hand-written DFA Lexer
Instead of using generator tools like Lex/Flex, the lexer was implemented completely by hand using a Deterministic Finite Automaton (DFA) approach in `src/lexer.c`.
- **State Transition Logic:** The lexer transitions between well-defined states (`STATE_START`, `STATE_IDENTIFIER`, `STATE_INT_LIT`, `STATE_STRING_LIT`, and operator lookaheads for two-character tokens like `==`, `!=`, `<=`, `>=`, `&&`, `||`).
- **Precise Error Reporting:** Two tracking counters (`line` and `col`) are updated character-by-character. Whenever an illegal symbol or unterminated string literal is detected, an explicit error message specifying the exact line and column is printed.
- **Roll-Derived Keyword Mapping:** Keywords are transformed using the rule: reversing the standard English keyword name and appending `42` (the last two digits of roll number `241030042`), such as `tcurts42` for `struct` and `noitcnuf42` for `function`.

### 1.2 Recursive-Descent LL(1) Parser & AST
The grammar was designed from scratch to be LL(1) parseable without backtracking.
- **Left-Recursion Elimination:** Standard arithmetic grammar has left recursion ($E \to E + T$), which induces infinite recursion in recursive-descent parsers. All arithmetic and logical productions were rewritten into iterative/tail-recursive LL(1) forms ($E \to T E'$, $E' \to + T E' \mid \epsilon$).
- **Left-Factoring:** Ambiguities such as the dangling `else` in `fi42 ... esle42` and postfix expressions (`id`, `id(...)`, `id.field`) were factored out using explicit optional tail productions (`ElseOpt`, `PostfixOpt`).
- **AST Architecture:** The Abstract Syntax Tree is implemented via tagged unions in `ASTNode`, cleanly modeling declarations, statements, binary/unary expressions, struct definitions, and nested functions.

### 1.3 Static Scoping & Symbol Table Architecture
Variant 2 requires nested functions with static (lexical) scoping.
- **Scope Hierarchy:** The symbol table is structured as an upward-pointing tree of `Scope` objects. Each scope maintains a `parent` pointer representing its enclosing lexical environment.
- **Lexical Resolution:** When resolving an identifier, `scope_lookup` searches the current scope's symbol list. If not found, it traverses upward along the `parent` pointer chain until either the symbol is found or the global scope is reached. This directly enforces static scoping rules for nested functions.
- **Semantic Error Checking:** The semantic analyzer performs type propagation and detects five distinct semantic errors: (1) Undeclared identifiers, (2) Duplicate declarations in the same scope, (3) Assignment and arithmetic type mismatches, (4) Invalid struct field access, and (5) Function return type mismatches.

### 1.4 Intermediate Representation (TAC) & Optimizations
- **Three-Address Code (TAC):** Expressions and high-level control structures are linearized into simple instructions of at most three operands (`res = arg1 op arg2`). This separates language-specific syntax from code generation.
- **Constant Folding:** A peephole optimization pass inspects binary operations with known integer literals and evaluates them at compile-time (e.g. `t1 = 10 + 20` $\to$ `t1 = 30`).
- **Dead Code Elimination (DCE):** Eliminates unreachable instructions immediately following unconditional control transfers (`goto` and `return`), as well as unused temporary variables.

### 1.5 Backend: Stack-Machine & Virtual Machine Runner
- Rather than emitting complex architecture-dependent assembly with system ABI overhead, the backend lowers TAC into stack-machine bytecode instructions (`PUSH_INT`, `STORE_VAR`, `LOAD_VAR`, `ADD`, `CALL`, `RET`, `ALLOC_STRUCT`, `GET_FIELD`, `SET_FIELD`).
- An execution engine simulates the stack machine with an explicit activation frame call-stack and heap-allocated records for structs.

---

## 2. A Bug Encountered & Debugging Process

### The Bug: Nested Function Scope Resolution and Calling Convention
During the initial implementation of Variant 2, executing `test06_nested_functions.ml` produced two severe issues:
1. **Semantic Bug:** The compiler threw a semantic error stating that local variables declared in the outer function were "undeclared" inside the nested function.
2. **Runtime Bug:** When calling a nested function or a function with parameters, the parameters evaluated to `0` and returned incorrect computation results.

### Root Cause Analysis:
1. **Scope Timing Issue:** In the initial parser implementation, nested function declarations (`nested_funcs`) were stored in a separate list from regular statements. During semantic analysis, the compiler checked all nested functions *before* checking the surrounding function's body statements. Consequently, local variables defined before the nested function were not yet inserted into the outer function's scope when the nested function was checked.
2. **Calling Convention Mismatch:** In the stack machine, the caller pushed arguments left-to-right (`param a1`, `param a2`), putting `a2` on the top of the operand stack. However, the callee was not popping the stack items into its local activation record upon entry, leaving parameter values uninitialized.

### How It Was Debugged & Resolved:
1. **Unified AST Sequence:** The parser was restructured to store nested function declarations directly within the function body's linear statement block (`body->as.block.statements`). During semantic analysis, statements and nested functions are traversed in strict textual order, allowing nested functions to see previously declared local variables via the `parent` scope link.
2. **Stack Callee Parameter Binding (`TAC_PARAM_POP`):** We introduced a `TAC_PARAM_POP` instruction. At function entry, the compiler pops values from the stack in reverse parameter order into the callee's frame variables (adhering to standard `cdecl` calling convention).
3. **Execution Skip Jump:** To prevent linear fall-through execution of nested functions within the outer function's body, the compiler emits an unconditional jump (`goto L_skip`) around nested function definitions.

After these fixes, `test06_nested_functions.ml` and `test10_complex_integration.ml` compiled cleanly and produced exact outputs.

---

## 3. One Thing I Would Change

If I were to redesign or extend this compiler, the primary change I would make is **implementing a register allocator (Linear Scan or Graph-Coloring) to emit native x86-64 machine assembly instead of an interpreted Stack-Machine Bytecode**.

While the stack-machine bytecode interpreter is highly portable and easy to verify, emitting real x86-64 assembly instructions (or RISC-V) would allow the compiler to interface directly with operating system system calls and standard C libraries without requiring an interpreter runtime. In addition:
- Struct records are currently allocated on a simple dynamic heap without garbage collection. Adding a mark-and-sweep or reference-counting garbage collector would prevent memory leaks during long-running programs.
