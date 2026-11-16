# Native entry and label-transfer conformance

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
The backend registers every generated native entry and every IR control-label
name. Native entry calls must disappear and Ama controls must remain labels,
so any remaining exact entry symbol or compiler-derived family is rejected. Missing/failed
object inspection is not success. The retained project pipeline performs this check before linking or publication;
[structured lowering](typed-plans.md) and
[independent generated contexts](generated-contexts.md) use that same verifier.
[Parallel execution](parallel-context-execution.md) tests actual overlapping
dispatch without changing the native entry disappearance requirement.

This relies on the selected compiler, object tooling and the stated no-renaming/
no-address-escape contract. It is not a sandbox for arbitrary malicious C, stripped
objects, inline assembly or a proof about all C compilers.

## Exact listing check

`src/native/toolchain/symbols.h` exposes the pure bounded byte-view checker
`ama_check_entry_symbols`. Forbidden entries include native entries and exact
IR control-label names; they are nonempty generated C identifiers; inputs are borrowed and not retained.
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
- A noninline native entry compiles but is rejected by the symbol checker.
- An Ama-shaped body implemented/called as a C function is rejected at both
  optimization levels, while the positive bodies remain labels.
- The exact compiler/target/C-mode header diagnostics reject deliberately
  unsupported configurations; the matching profile probe compiles and executes.
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
helper calls and avoid relying on sibling-call optimization. Native entry expansion
is mandatory; ordinary helper inline hints remain optional under STACK-02/STACK-04.


T148 adds source-linked contracts for target/compiler assertions, nm record/token
recognition and exact entry-family checks. A clear result is meaningful only for
the declared GCC 15.2.0, C11, x86_64 Linux, unstripped non-LTO object workflow.
The listing recognizer accepts its documented bounded token grammar; it is not a
general parser for arbitrary debugger/assembler output. Required inlining plus
actual positive/negative compiled probes establish this supported execution
boundary; symbol absence alone does not prove functional correctness or native
borrow/resource behavior. Ordinary C helper recursion remains permitted.

## One enforced compilation profile

T008 adds `src/native/toolchain/profile.h`: one fixed argv list shared by real
project compilation, its compilation database and the native acceptance harness.
It requires C11, pedantic errors, Wall/Wextra, Werror including attribute errors,
non-LTO and non-whole-program compilation. Optimization remains separate: the
project backend selects O2; the acceptance harness verifies both O0 and O2.
The generated native header still enforces GCC 15.2.0, x86_64 Linux and the exact
representation assumptions. This does not constrain the compiler's own host build.
Native-authoring syntax-only tooling is an editor aid, not a substitute for this
object verification workflow.

Required project flags follow user flags. Consequently user Wno-error, flto and
C99 options cannot silently weaken the supported compilation boundary. The argv
capacity is derived from the actual fixed array including its terminator, with
checked addition to user flag count. There is no magic command length or shell
reconstruction. An independent JSON consumer checks mandatory flag order and
compiles the actual published launcher using its recorded argument array. A
separate unused native helper verifies that warnings still reject compilation
when a user requested Wno-error; rejection leaves no new executable artifact or
publication authority.

## Verify exact Ama control identities

`codegen/toolchain/verify.c` owns tool execution, complete record validation and
unstripped main evidence. `verification/entries.c` then checks native identities
and every exact control selector from resolved IR. The selector spelling is the
emitter's `ama_label_<decimal selector>`, not an arbitrary name-prefix guess.
A native/helper symbol merely sharing a longer identifier remains unrelated.
The fixed local name buffer includes the prefix, terminator and size_t bit count;
decimal digit count cannot exceed that bit count, proving the bound over the
whole selector domain. No selector is truncated or hashed to guess a match.

The finite type/control inventory and finite listing bound all loops. This
composition conservatively rescans the listing for each registered entry, costing
O(K * (B + S*L)) for K native/control entries, B listing bytes, S symbols and
maximum name length L; selector scratch is O(size_t bit count). Native-name
concatenations belong to the scaffold arena. There is no claim of constant-time
verification or a general proof about malicious native C with renamed symbols,
indirect dispatch, inline assembly or unsupported object tooling. The generator
emits only registered labels and the native authoring contract forbids escaping or
renaming entries; the evidence applies to that explicit supported boundary.

The real generated-execution leaf additionally injects a C function with an exact
IR selector name. C permits a function and a block-local label with that same
spelling, so compilation alone accepts it; object verification rejects it before
linking or executable publication. This complements the standalone negative
C-call dispatcher and the positive ordinary recursive C helper. Tests distinguish
forbidden Ama entry boundaries from permitted native/helper stack use.

Generated view binders are ordinary static C helpers, marked unused when a type
needs no such view. They may use C calls and may be optimized normally under
STACK-04. Native entry uses always_inline plus mandatory actual symbol inspection.
The profile does not globally enable Winline: failure to honor an ordinary inline
hint is not an Ama boundary violation. An ordinary inline-hinted helper containing
local setjmp is retained as a real function and executes normally at O0/O2, while
the native entry containing setjmp directly is rejected as uninlinable. The local
helper never performs a nonlocal exit or retains its automatic jump buffer.
These paired probes distinguish mandatory entry expansion from permitted C stack
use without relaxing the actual native entry check.

T008 acceptance passes the strict full suite with `-DNDEBUG -Werror`, O0/O2
native toolchain probes, and the symbol/generated-execution leaves under
ASan/UBSan with leak detection disabled. Documentation coverage checks all 700
maintained inputs with zero debt. Two warning-free Doxygen generations render
536 file pages, 2229 documented interfaces and all 285 canonical/page destinations
with identical complete output bytes. These are completion evidence counts.
