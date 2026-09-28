# MiniLang Compiler Lab Report

**Student Name:** Md Arzan Molla  
**Roll Number:** 241030042  
**Assigned Variant:** Variant 2 (241030042 % 4 = 2 -> Structs/records and nested functions with static scoping)  
**Course:** Compiler Design Sessional  

---

## 1. Design Decisions

For this lab project, I had to build a working compiler front-end and back-end for MiniLang. Because my roll is 241030042, my remainder modulo 4 is 2. So my required language features are user-defined structs (records) and nested functions with static scoping.

Here are the main design choices I made while building each phase:

### 1.1 Hand-written Lexer (DFA)
Our teacher asked us not to use Lex or Flex. I wrote the lexer from scratch in `src/lexer.c` using a basic DFA approach. It reads the source code character by character and tracks `line` and `col` numbers so that if someone types an invalid symbol or forgets to close a string quote, it prints the exact line and column number. 
For keywords, my roll ends in 42, so I reversed the normal English words and added 42 at the end (for example, `struct` becomes `tcurts42`, `function` becomes `noitcnuf42`, `let` becomes `tel42`, and `print` becomes `tnirp42`).

### 1.2 Recursive-Descent Parser & AST
I used recursive-descent parsing because it is easy to write by hand and debug.
- **Removing Left Recursion:** The standard expression grammar `E -> E + T | T` cannot be parsed with recursive descent because the function calls itself without consuming any input, causing infinite recursion. I removed left recursion by splitting it into `E -> T E'` and `E' -> + T E' | epsilon`.
- **Left Factoring:** For `if` and `if-else` statements, both start with `fi42 (condition)`. To avoid backtracking with 1 token lookahead, I factored out the optional `esle42` part into an `ElseOpt` helper rule.
- **AST Representation:** In `src/ast.c` and `src/ast.h`, I used C structs with tagged unions (`ASTNode`) to store nodes like variable declarations, struct definitions, if-statements, function calls, and binary expressions.

### 1.3 Symbol Table and Static Scoping
Because Variant 2 requires nested functions, the compiler must support lexical scoping. I built the symbol table as a tree of scopes in `src/symbol_table.c`. Each `Scope` struct has a `parent` pointer. 
When a variable is looked up inside a nested function, the lookup function first checks the local scope. If it is not found there, it follows the `parent` pointer up to the enclosing function's scope. This allows inner functions to read and write variables from outer functions.
I also wrote semantic checks in `src/semantic.c` to catch 5 specific errors:
1. Using an undeclared variable or function.
2. Declaring the same variable twice in the same scope.
3. Assigning the wrong type (like putting a string into an int variable).
4. Accessing a struct field that doesn't exist.
5. Returning the wrong type from a function.

### 1.4 Intermediate Code (TAC) and Optimizations
Instead of going straight to assembly, I generated Three-Address Code (TAC) in `src/tac.c`. This breaks complex expressions into simple lines with temporary variables (like `t1 = a + b`).
On top of TAC, I wrote two simple optimizations in `src/optimizer.c`:
- **Constant Folding:** If both operands in an operation are numbers (like `10 + 20`), the optimizer calculates `30` at compile time so the runtime doesn't have to compute it.
- **Dead Code Elimination:** Any instructions placed right after an unconditional `return` or `goto` (before the next label) can never run, so the optimizer removes them.

### 1.5 Backend and Virtual Machine
To execute the program, I wrote a small stack machine in `src/vm.c`. The compiler translates TAC into bytecode instructions like `PUSH_INT`, `STORE_VAR`, `LOAD_VAR`, `ADD`, `CALL`, and `RET`. A virtual machine runner then loops through these instructions with a call stack and executes the program. For structs, it allocates dynamic objects and gets/sets fields by name.

---

## 2. A Bug I Encountered and How I Debugged It

### The Problem
When I started testing nested functions with `tests/test06_nested_functions.ml`, I ran into an annoying bug. In that test, an outer function `compute` has a local variable `tel42 tni42 factor = 5;` and then defines a nested function `noitcnuf42 helper(...)` which uses `factor`.

When I compiled it, two things went wrong:
1. The semantic checker printed an error saying `factor` was an undeclared identifier inside `helper`.
2. Even when I tried running it, when functions took parameters, the parameters showed up as `0` inside the function body.

### Debugging Steps
I opened `src/parser.c` and `src/semantic.c` and added print statements to see what was happening.
1. First, I found that my parser was storing all nested function declarations in a separate list called `nested_funcs`, outside the normal list of body statements. In `semantic.c`, my code was checking `nested_funcs` before checking the regular statements of the outer function. Because `factor` was declared as a statement in the outer function, it hadn't been added to the symbol table yet when `helper` was being checked!
2. Second, in my stack machine backend, when calling a function with arguments `a` and `b`, the caller pushed `a` and then `b`. But upon entering the function, I hadn't written any code to pop those values off the stack into the parameter variables. So the parameters stayed uninitialized (zero).

### The Fix
1. In `src/parser.c`, I changed the parser so that nested functions are kept in the exact same statement list as other statements. That way, the semantic checker visits `factor = 5;` first, registers it in the outer scope, and then visits the nested function `helper`. Since `helper`'s scope points to the outer scope as its parent, it finds `factor` easily.
2. In `src/tac.c`, I added a `TAC_PARAM_POP` instruction. When a function starts, it pops values off the stack in reverse order and stores them into the parameter variables.
3. I also added a jump (`goto L_skip`) around nested function bodies so that when the outer function runs linearly, it doesn't accidentally fall into the nested function's code.

After making these changes, `test06_nested_functions.ml` printed `25` as expected, and `test10_complex_integration.ml` worked properly too.

---

## 3. One Thing I Would Change

If I had more time to work on this compiler, the main thing I would change is the backend code generator. Right now, it outputs stack-machine bytecode that runs inside an interpreter. 

While this was straightforward to implement and debug for the lab, an interpreted stack machine is much slower than native code. If I were doing this again, I would write an x86-64 code generator that outputs actual GNU assembler (`.s`) files. That would allow the programs to be compiled with `gcc` into real standalone executables on Windows or Linux, and I could learn how CPU registers and real calling conventions work. I would also add a simple garbage collector for struct instances, because right now heap structs are allocated with `calloc` but never freed during execution.
