# GRAMMAR.md - MiniLang Grammar Specification

**Roll Number:** 241030042  
**Variant:** Remainder 2 (Structs/records, plus nested functions with static scoping)  
**Parser Model:** Hand-written LL(1) / Recursive-Descent Parser

---

## 1. Left-Recursion Elimination

Standard expression grammars are often defined with left-recursive rules such as:

```text
E -> E + T | E - T | T
T -> T * F | T / F | F
```

Direct top-down (recursive-descent) parsing cannot handle left-recursion because it causes infinite recursion. We eliminated left-recursion using the standard transformation:

$$A \to A\alpha \mid \beta \implies A \to \beta A', \quad A' \to \alpha A' \mid \epsilon$$

### Transformed Rules:
```text
Expr        -> LogAnd ExprTail
ExprTail    -> '||' LogAnd ExprTail | ε

LogAnd      -> Equality LogAndTail
LogAndTail  -> '&&' Equality LogAndTail | ε

Equality      -> Relational EqTail
EqTail        -> ('==' | '!=') Relational EqTail | ε

Relational    -> Additive RelTail
RelTail       -> ('<' | '<=' | '>' | '>=') Additive RelTail | ε

Additive      -> Multiplicative AddTail
AddTail       -> ('+' | '-') Multiplicative AddTail | ε

Multiplicative-> Unary MulTail
MulTail       -> ('*' | '/') Unary MulTail | ε
```

---

## 2. Left-Factoring

Left-factoring is applied where two production alternatives share a common prefix, eliminating ambiguity for the 1-token lookahead ($LL(1)$).

### A. Conditional Statement (`fi42 ... esle42`)
Original:
```text
IfStmt -> 'fi42' '(' Expr ')' Stmt 'esle42' Stmt
        | 'fi42' '(' Expr ')' Stmt
```
Left-Factored:
```text
IfStmt    -> 'fi42' '(' Expr ')' Stmt ElseOpt
ElseOpt   -> 'esle42' Stmt | ε
```

### B. Primary Expression / Calls / Field Access
Original:
```text
Primary -> IDENTIFIER
         | IDENTIFIER '(' ArgList ')'
         | IDENTIFIER '.' IDENTIFIER
```
Left-Factored:
```text
Primary     -> IDENTIFIER PostfixOpt
PostfixOpt  -> '(' [ ArgList ] ')' PostfixOpt
             | '.' IDENTIFIER PostfixOpt
             | ε
```

---

## 3. Complete Grammar (in EBNF)

```text
Program         ::= { StructDecl | FuncDecl | VarDecl } EOF

StructDecl      ::= 'tcurts42' IDENTIFIER '{' { VarDecl } '}'

FuncDecl        ::= 'noitcnuf42' IDENTIFIER '(' [ ParamList ] ')' ':' Type '{' { FuncDecl } { Stmt } '}'
ParamList       ::= Param { ',' Param }
Param           ::= Type IDENTIFIER

VarDecl         ::= 'tel42' Type IDENTIFIER [ '=' Expr ] ';'

Type            ::= 'tni42' | 'loob42' | 'gnirts42' | 'diov42' | IDENTIFIER

Stmt            ::= VarDecl
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
