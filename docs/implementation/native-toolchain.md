# Native entry and label-transfer conformance

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

This records the tested toolchain boundary under STACK-01 through STACK-04 and
GEN-01 through GEN-03. It is an executable backend constraint harness, not a
source-language frontend or a completed runtime.

## Enforcement and assumptions

Use the pinned [toolchain profile](native-abi.md). `AMA_NATIVE_ENTRY` forces direct
entry expansion; GCC must reject a direct call it cannot inline. Entry addresses,
indirect entry calls, native entry assembler renaming, and retained entry pointers
are outside the native contract. Plain inline/LTO alone are not accepted evidence.

The checked artifact is an unstripped, non-LTO object from one generated project
translation unit. After a successful compile, invoke GNU `nm --format=posix` for
that one object and require successful tool exit and complete output capture.
The backend must register every generated native entry name. It must reject any
remaining entry symbol or compiler-derived entry symbol family. Missing/failed
object inspection is not success. Generator integration is a later codegen task.

This relies on the selected compiler, object tooling and the stated no-renaming/
no-address-escape contract. It is not a sandbox for arbitrary malicious C, stripped
objects, inline assembly or a proof about all C compilers.

## Exact listing check

`src/native/toolchain/symbols.h` exposes a pure bounded byte-view checker. Entry
names are nonempty generated C identifiers; inputs are borrowed and not retained.
The listing is a sequence of nonempty records separated by LF, optionally without
its final LF. A record has two through four space/tab-separated words: symbol,
one ASCII alphabetic or `?` type character, then optional hexadecimal value/size.
Leading/trailing horizontal space is accepted; control bytes are not. This is the
lexical format consumed from the pinned tool, not a general object-file parser.

Each record must validate before its symbol is checked. A symbol belongs to an
entry if it equals its name or starts with that complete name followed by `.`,
the compiler-derived suffix separator. A longer C identifier such as `entry_extra`
is unrelated. A malformed record yields Malformed, any entry-family symbol yields
EntryRemains, otherwise Clear. The whole stream is validated even after a match,
so malformed trailing data cannot be certified. Empty output is valid only as a
successfully inspected object with no listed symbols.

The scanner's position increases within bounds; each extracted word is nonempty.
Induction over records proves every symbol is checked and no later malformed
record is skipped. Full-name comparison and the illegal-in-C-name dot separator
avoid guessing from basenames or arbitrary substrings. For B listing bytes,
S symbols, K entries and maximum entry-name length L, work is O(B + S*K*L),
auxiliary storage O(1). It allocates nothing and modifies no input.

## Executable evidence

Run `make -C tests/unit/native/toolchain/symbols` and
`make -C tests/integration/native/toolchain`.

Unit tests cover unrelated prefixes, direct/undefined entry symbols, compiler
suffixes, empty output, horizontal whitespace and malformed fields. Integration
uses real GCC and nm at both `-O0` and `-O2`:

- A forced-inline native entry disappears from the object symbol table. The
  resulting program links and executes correctly while a deliberately noninline,
  recursive C helper computes its native result.
- A noninline entry compiles but is rejected by the symbol checker.
- A fixed-signature entry containing `setjmp` cannot be inlined and must fail
  compilation. It never executes a nonlocal exit or bypasses Ama cleanup.
- An incompatible typed index helper binding fails C compilation.

The positive project keeps instance data and control selectors in explicit heap/
runtime storage. Its two Ama-shaped bodies are labels in `main`, each emitted
once; entry uses `goto`, return uses a validated `switch` selector. No C function
exists for either body. The native helper's C recursion is intentional and allowed.
The fixture verifies the code shape and result routing, not future generation,
interface reset, pool growth or the complete runtime acquisition state machine.

The probe uses additional optimization controls to preserve observable native
helper calls and avoid relying on sibling-call optimization. An initial probe with
blanket `-fno-inline` plus `-Werror=inline` rejected ordinary optional inline helpers;
that blanket restriction is not used. Only native entry expansion is mandatory.
