# KEYWORDS.md - MiniLang Keyword Mapping

**Student Roll Number:** 241030042  
**Variant:** 241030042 mod 4 = 2 (Structs/records, plus nested functions with static scoping)  
**Keyword Derivation Rule:**  
Every standard programming keyword is transformed by **reversing the English word** and appending the suffix **`42`** (derived from the last two digits of roll number `241030042`).

---

### Keyword Mapping Table

| Conventional Keyword | Roll-based Keyword (`<reversed>42`) | Purpose / Description |
| :--- | :--- | :--- |
| `struct` | `tcurts42` | Declares a user-defined record / struct type |
| `function` | `noitcnuf42` | Declares a top-level or nested function |
| `return` | `nruter42` | Returns a value or exits from a function |
| `if` | `fi42` | Conditional branching statement |
| `else` | `esle42` | Alternative branch in conditional statement |
| `let` | `tel42` | Declares a local or global variable |
| `int` | `tni42` | 32-bit integer primitive type |
| `bool` | `loob42` | Boolean primitive type (`eurt42` / `eslaf42`) |
| `string` | `gnirts42` | String literal primitive type |
| `void` | `diov42` | Return type for functions returning no value |
| `true` | `eurt42` | Boolean true literal |
| `false` | `eslaf42` | Boolean false literal |
| `print` | `tnirp42` | Built-in output statement |
| `new` | `wen42` | Allocates an instance of a struct |

---

### Example Code Snippet

```minilang
tcurts42 Point {
    tel42 tni42 x;
    tel42 tni42 y;
}

noitcnuf42 main(): diov42 {
    tel42 Point p = wen42 Point;
    p.x = 10;
    p.y = 20;

    noitcnuf42 add_coords(): tni42 {
        nruter42 p.x + p.y;
    }

    tel42 tni42 sum = add_coords();
    tnirp42(sum);
}
```
