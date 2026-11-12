# Grammar expectations and frontend composition

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

Boundary: `src/parser/error/`, integrated through sections, AST parsing and ABI
records. Describes COMP-01/04/05 syntax diagnostics for the tested subset without binding
or generated C. `ParseError` combines a stable ParserExpectation key and the
existing structured Diagnostic. Text is derived from the key, not parsed back
into a machine representation. The table is compile-time checked for coverage.

Each grammar failure branch selects its expected construct explicitly. Examples:
missing type -> same-indent module path; unterminated index -> ]; literal
assignment operand -> copy source; entry >> -> input/output execution bindings; missing
lifecycle record -> the mandatory ABI symbol. These are grammar predicates, not
edit-distance guesses, symbol-name suggestions or alias-dependent alternatives.
Help is added only for exact structural rules. Source UTF-8/whitespace and memory
failures enter the same error representation; no allocation is required to report
another allocation failure. Related duplicate-binding spans remain independent
structured notes with both physical locations.

Access errors point at the failing byte or EOF rather than an entire unrelated
statement. A leading dot at the same indentation after a declaration-name line
selects the type-line production even when that type spelling is malformed;
validation reports the failed type requirement, without retrying another semantic
interpretation. Literal payload cannot activate a file or introduce nested bindings. Only its
explicit declaration/input marker determines its syntactic role.
A declared or undeclared name has exactly the same syntactic interpretation.

The recognizer's invariant is a monotone source cursor plus explicit nesting
frames and finite argument/section states. Every production advances, changes a
frame state once, pops a frame, or reports failure. The component contracts derive
correctness/complexity for these transitions:
[layout](layout.md), [sections](parser-sections.md), [AST](syntax-tree.md),
[calls](call-bindings.md), and [native records](abi-parser.md). Syntax returns an
owned, source-mapped tree or an invalid tree with error; it never resolves names
or publishes generated executable output.

The [test tree](../../tests/unit/parser/Makefile) exposes independently runnable
section, access, AST, call, native-table and error suites. Access recognition is
compared to an independent bounded recursive grammar oracle for all 21,845
strings of length zero through seven over `a&[]`. Call modes are compared to a
finite-language model for all 5856 short binding sequences. These finite models
exercise boundary partitions; the general argument does not assume their samples
exhaust all programs. Tree tests include 512-level nesting, retained literal
spaces/NUL/CRLF, and every allocation refusal in a composed parse. Prefix tests
exercise every byte truncation of a Unicode program, including split UTF-8.

The [integration leaf](../../tests/integration/parser/grammar/main.c) loads real
extensionless files/native companions, parses the AST, checks handwritten output
expectations, renders syntax/ABI errors with original spans, and proves that a
native companion does not activate a draft. Its native-binding probe emits typed
uses from preserved module-local ABI spellings, qualifies them with registered
module prefixes, and compiles/runs two companions with identical local names in
one C translation unit. It exercises Unicode, UCN and GCC-dollar names and all six hook signatures.
This is acceptance evidence for metadata handoff, not production C generation.

Verification performed:

- Full `make -j4 test` with C11/pedantic, Werror and NDEBUG passed. Test recipes
  restore assertions through UNDEBUG.
- Lexer, parser, native namespace/resource and composed integration suites passed ASan/UBSan with
  `ASAN_OPTIONS=detect_leaks=0` and non-PIE builds. LeakSanitizer remains unsupported
  under environment ptrace supervision; no leak certification is claimed.
- Ten frontend headers compile independently; repository document links and C
  formatting are checked. Explicit scenarios cover the frontend leaves,
  including the finite-domain and allocation-failure loops above.
- Editor/Git fixture policies preserve payload trailing spaces and deliberate
  endings. C files retain four-space formatting and ordinary LF conventions.

Type compatibility, actual binding arity, namespace uniqueness, initialization
order/cycles, required-draft dependencies and native callable/signature checking
remain their resolution/build stages. A syntactically valid AST may fail them.
The command-line executable remains the explicit implementation foundation until
shared check/build commands are implemented. The revised CALL-07 grammar is implemented at the source/AST boundary;
constructor execution and semantic capability checking remain later work.
