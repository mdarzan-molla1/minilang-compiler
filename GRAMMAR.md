# GRAMMAR.md - MiniLang Grammar Specification

**Roll Number:** 241030042  
**Variant:** Variant 2 (Structs/records and nested functions with static scoping)  
**Parser Type:** Hand-written Recursive-Descent / LL(1) Parser  

---

## 1. Removing Left Recursion

In a recursive-descent parser, we cannot directly use left-recursive rules like:

```text
E -> E + T | E - T | T
T -> T * F | T / F | F
```

If we do, the `parse_expression()` function will keep calling itself before reading any token from the input, causing an infinite loop (stack overflow). 

To fix this, I transformed the grammar using the standard textbook formula:
$$A \to A\alpha \mid \beta \implies A \to \beta A', \quad A' \to \alpha A' \mid \epsilon$$

### Transformed Rules in MiniLang:
```text
Expr            -> LogAnd ExprTail
ExprTail        -> '||' LogAnd ExprTail | ε

LogAnd          -> Equality LogAndTail
LogAndTail      -> '&&' Equality LogAndTail | ε

Equality        -> Relational EqTail
EqTail          -> ('==' | '!=') Relational EqTail | ε

Relational      -> Additive RelTail
RelTail         -> ('<' | '<=' | '>' | '>=') Additive RelTail | ε

Additive        -> Multiplicative AddTail
AddTail         -> ('+' | '-') Multiplicative AddTail | ε

Multiplicative  -> Unary MulTail
MulTail         -> ('*' | '/') Unary MulTail | ε
```

---

## 2. Left Factoring

Left factoring is needed when two or more choices in a rule start with the exact same token. Since this parser only looks 1 token ahead (LL(1)), it cannot decide which branch to take without backtracking unless we factor out the common prefix.

### A. If-Else Statements
Original rule:
```text
IfStmt -> 'fi42' '(' Expr ')' Stmt 'esle42' Stmt
        | 'fi42' '(' Expr ')' Stmt
```
Both start with `fi42 (Expr) Stmt`. Left-factored version:
```text
IfStmt    -> 'fi42' '(' Expr ')' Stmt ElseOpt
ElseOpt   -> 'esle42' Stmt | ε
```

### B. Identifiers, Calls, and Field Access
When the parser sees an identifier, it could be a simple variable (`x`), a function call (`foo(a, b)`), or a struct field access (`pt.x`). Left-factored version:
```text
Primary     -> IDENTIFIER PostfixOpt
PostfixOpt  -> '(' [ ArgList ] ')' PostfixOpt
             | '.' IDENTIFIER PostfixOpt
             | ε
```

---

## 3. Full Grammar (EBNF)

```text
Program         ::= { StructDecl | FuncDecl | VarDecl } EOF

StructDecl      ::= 'tcurts42' IDENTIFIER '{' { VarDecl } '}'

FuncDecl        ::= 'noitcnuf42' IDENTIFIER '(' [ ParamList ] ')' ':' Type '{' { Stmt } '}'
ParamList       ::= Param { ',' Param }
Param           ::= Type IDENTIFIER

VarDecl         ::= 'tel42' Type IDENTIFIER [ '=' Expr ] ';'

Type            ::= 'tni42' | 'loob42' | 'gnirts42' | 'diov42' | IDENTIFIER

Stmt            ::= VarDecl
                  | FuncDecl
                  | IfStmt
                  | AssignStmt
                  | PrintStmt
                  | ReturnStmt
                  | Block
                  | ExprStmt

IfStmt          ::= 'fi42' '(' Expr ')' Stmt [ 'esle42' Stmt ]
AssignStmt      ::= ( IDENTIFIER | IDENTIFIER '.' IDENTIFIER ) '=' Expr ';'
PrintStmt       ::= 'tnirp42' '(' Expr ')' ';'
ReturnStmt      ::= 'nruter42' [ Expr ] ';'
Block           ::= '{' { Stmt } '}'
ExprStmt        ::= Expr ';'

Expr            ::= LogAnd { '||' LogAnd }
LogAnd          ::= Equality { '&&' Equality }
Equality        ::= Relational { ( '==' | '!=' ) Relational }
Relational      ::= Additive { ( '<' | '<=' | '>' | '>=' ) Additive }
Additive        ::= Multiplicative { ( '+' | '-' ) Multiplicative }
Multiplicative  ::= Unary { ( '*' | '/' ) Unary }

Unary           ::= ( '-' | '!' ) Unary 
                  | Primary

Primary         ::= INT_LIT
                  | STRING_LIT
                  | 'eurt42'
                  | 'eslaf42'
                  | 'wen42' IDENTIFIER
                  | '(' Expr ')'
                  | IDENTIFIER [ '(' [ ArgList ] ')' ] { '.' IDENTIFIER }

ArgList         ::= Expr { ',' Expr }
```
