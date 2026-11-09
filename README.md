# CIE2Py

## TL;DR

CIE2Py is a small C++20 transpiler that takes Cambridge IGCSE pseudocode (0478 / 0984) and turns it into Python 3 you can actually run. Feed it a `.cie` file, get a `.py` file. It also has an `--ast` mode for staring at the parse tree when the output looks wrong.

**This is just a toy project** — a student's daydream, not a product. It is version 0.1.0, which is a polite way of saying "early". It is not a Cambridge product, not an exam oracle, and not a substitute for learning the pseudocode. It is a small, readable transpiler that does the boring transcription for you.

## Why CIE2Py?

### Why this is a problem

Pseudocode is a strange artefact: precise enough to be marked, too inert to be run. Every time I want to test an algorithm, I end up translating it into Python by hand. Declarations become assignments. Arrays grow an offset. `DIV` and `MOD` need a second thought. `OUTPUT` loses its commas. None of this is the interesting part of the problem, and every manual translation is another chance to introduce a bug that was never in the algorithm.

The obvious answer is a converter. The less obvious problem is that most converters are black boxes. They are either a website that swallows the source and returns something almost right, or a dependency tree that does a hundred things I did not ask for. When the output is wrong, I cannot tell which stage is wrong.

### The current idea

So the idea is **transparency**, not intelligence. Lexer, parser, AST, code generator: four core pieces, each with one job, each small enough to read. A small CLI glues them together. No plugin system, no language server, no "smart" inference, no 30 MB dependency tree. If `DIV` is wrong, the fix belongs in one helper function, not in a prompt. The tests live in `tests/` and are wired into CTest, so the thing is at least allowed to disagree with me in public.

I have no illusion that this is a finished product. It is a small, dry slab of AI slop that happens to compile. There is no semantic analysis; the Python interpreter is the type checker. I do not know yet whether it will be useful in practice, or whether it is just another way to avoid doing the actual work. But I would rather find out with a small tool I can read end to end than with a black box.

If you want something that guesses at your homework, this is not it. If you want a tiny, predictable thing that shows its work, maybe it is. I will keep poking at it.

## Function

CIE2Py is deliberately a straight pipeline, built from small pieces with one job each:

| Stage | Location | Responsibility |
| --- | --- | --- |
| Lexer | `src/lexer/` | Turns source text into tokens. Handles `//` comments, newlines, literals, and both `<-` and `←` for assignment. |
| Parser | `src/parser/` | Recursive-descent parser. Builds the AST, handles operator precedence, and reports the first syntax error at the offending token. |
| AST | `src/ast/` | Node definitions plus a human-readable AST printer used by `--ast`. |
| Code generator | `src/converter/` | Walks the AST and emits standalone Python 3, including any helper functions the program needs. |
| CLI | `src/main.cc` | Argument parsing, file/stdin input, file/stdout output, and diagnostics. |

### What it understands

- `DECLARE` for `INTEGER`, `REAL`, `CHAR`, `STRING`, and `BOOLEAN`
- `CONSTANT`
- Arrays with custom lower/upper bounds, including multi-dimensional arrays
- Assignment with `<-` or `←`
- Uppercase keywords, as used by CIE pseudocode
- `INPUT` and `OUTPUT`
- `IF` / `THEN` / `ELSE` / `ENDIF`
- `CASE OF` / `OTHERWISE` / `ENDCASE`
- `FOR` / `TO` / `STEP` / `NEXT`
- `WHILE` / `DO` / `ENDWHILE`
- `REPEAT` / `UNTIL`
- `PROCEDURE` / `ENDPROCEDURE`, `FUNCTION` / `ENDFUNCTION` / `RETURNS`
- `CALL` and `RETURN`
- `OPENFILE` / `READFILE` / `WRITEFILE` / `CLOSEFILE`
- Arithmetic, comparison, and logical operators, including `DIV`, `MOD`, and `^`
- Built-ins: `LENGTH`, `SUBSTRING`, `LCASE`, `UCASE`, `ROUND`, `RANDOM`
- `//` line comments

### The interesting mappings

- **Declarations become initialised variables.** Python has no declarations, so `DECLARE Score : INTEGER` becomes `Score = 0`. Arrays become nested lists.
- **Array lower bounds are translated.** `DECLARE A : ARRAY[1:10] OF INTEGER` becomes a list of ten elements, and `A[i]` becomes `A[i - 1]`. Negative and non-literal lower bounds are handled too.
- **`OUTPUT` does not invent spaces.** `OUTPUT Name, " passed"` becomes `_cie_print(Name, " passed")`, because the comma separates values; it is not a space. The helper keeps booleans as `TRUE` / `FALSE`.
- **`DIV` and `MOD` keep the syllabus meaning.** The generated helpers discard the fractional part, so negative operands truncate towards zero instead of following Python's floor behaviour.
- **`ROUND` and `RANDOM` support both generations of syntax.** `ROUND(x, places)` and `RANDOM()` work, as do the older one-argument forms.
- **Identifiers are case-insensitive.** `Score`, `score`, and `SCORE` map to the same Python name. Names that collide with Python keywords or generated helpers get a trailing underscore.
- **Subprograms that assign a new value to a global variable get a `global` statement.** The generator scans each procedure and function body before emitting it.

### Usage

```text
converter [options] <input.cie>
```

| Argument / option | Meaning |
| --- | --- |
| `<input.cie>` | Pseudocode source file. Use `-` to read from stdin. |
| `-o`, `--output <file>` | Write Python (or AST with `--ast`) to `<file>`. Default is stdout; `-` also means stdout. |
| `--output=<file>` | Same as `--output <file>`. |
| `--ast` | Print the parsed AST instead of Python. |
| `--` | Treat the next argument as the input file name, even if it starts with `-`. |
| `-h`, `--help` | Print the full usage text and exit. |

Examples:

```bash
# Write Python to stdout.
./build/converter program.cie

# Write Python to a file.
./build/converter program.cie -o program.py

# Inspect the AST instead.
./build/converter --ast program.cie

# Read pseudocode from stdin.
cat program.cie | ./build/converter -

# Full help.
./build/converter --help
```

Run `converter` with no arguments and it prints the full help and exits with status `1`. The same happens after a syntax error: you get `file:line:column: message`, then the usage text. A successful run exits with `0`.

### Example

Input (`program.cie`):

```text
DECLARE Score : INTEGER
DECLARE Name : STRING

Name <- "Ada"
INPUT Score

IF Score >= 50 THEN
  OUTPUT Name, " passed"
ELSE
  OUTPUT Name, " failed"
ENDIF
```

Output (`./build/converter program.cie`):

```python
# Generated by CIE2Py

def _cie_text(value):
    if value is True:
        return "TRUE"
    if value is False:
        return "FALSE"
    return str(value)

def _cie_print(*values):
    print("".join(_cie_text(value) for value in values))

Score = 0
Name = ""
Name = "Ada"
Score = int(input())
if Score >= 50:
    _cie_print(Name, " passed")
else:
    _cie_print(Name, " failed")
```

The generated file needs only the Python standard library. Any helper it uses (`DIV`/`MOD`, `ROUND`, `SUBSTRING`, text/CHAR formatting, dynamic step handling) is emitted into the file itself.

### Toy caveats

- There is no semantic analysis. Undeclared variables, type mismatches, and division by zero are Python's problem at runtime.
- The converter is syntax-directed. It translates what you wrote; it does not prove that what you wrote is correct.

## Installation Guide

Requirements:

- CMake 3.20 or newer
- A C++20 compiler (GCC, Clang, AppleClang, or MSVC)
- Python 3 for the integration tests and for running generated programs — the converter itself does not need Python at build time

Clone and build:

```bash
git clone https://github.com/xin1z/CIE2Py.git
cd CIE2Py
cmake -S . -B build
cmake --build build
```

Check that it works:

```bash
./build/converter --help
```

Run the integration tests (requires Python 3):

```bash
ctest --test-dir build --output-on-failure
```

There are no submodules, no vendored dependencies, and no package manager involved. If CMake and a C++20 compiler are available, the above is the whole installation procedure. If the build fails, check that your compiler actually supports C++20; CMake will not rescue a 2015 toolchain.

## License

CIE2Py is released under the GNU General Public License v3.0. See [LICENSE](LICENSE) for the full text.

Cambridge IGCSE is a trademark of Cambridge Assessment International Education. CIE2Py is not affiliated with or endorsed by them.

## BTW?

Yep! This README.md is basically human-brainstormed and AI-generated.

Feed the LLM a pile of my articles, yap for a bit, and ta-da! Done!

Let's build the slopplication together!
