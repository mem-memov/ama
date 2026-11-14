# Driver option assembly

T138 decomposes the existing CLI parser without changing its accepted language,
error precedence, ownership or request effects. The canonical
[compiler architecture](../../spec/implementation/compiler.md#arch-01) requires
small components and exact algorithms; this engineering contract records the
current CLI subset, not new Ama source semantics. Current path spelling still
belongs to the source resolver and its T014 adaptation.

## Boundaries and ownership

| Component | Responsibility and narrow interface | State/effects |
|---|---|---|
| `driver/options/recognition/` | Exact spelling, request applicability and value arity through `driver_option_recognize` | Immutable table; borrowed result; no allocation |
| `driver/options/storage/` | `DriverOptions` arena lifetime, bounded arrays, final root/exclusion publication | Own arrays; borrow all argv strings; cleanup includes partial allocation |
| `driver/options/validation/` | Operand/path checks, request-specific option relations, diagnostic category | No filesystem/toolchain effects; delegates logical spelling |
| `driver/options/assembly/` | Consume argv entries and install recognized values/flags | Append ordered root/exclusion/C-flag entries; update scalar settings |
| `driver/options/parse.c` | Compose children and publish only a fully validated request | No frontend, source loading or native execution |

The public options header depends on source/diagnostic/request records, not the
complete frontend. A command that needs a frontend includes that boundary itself.
Every header is independently includable; private children expose only their
actual collaborators. No component invents implicit roots or output exclusions.

Call init, parse once, inspect success or diagnostic, then destroy. The argument
count is nonnegative and all count entries are valid nonnull C strings; zero
permits NULL. Input strings remain alive until owner destruction. Parse failure
keeps `valid` false, preserves the diagnostic and retains partial arrays in the
arena for one ordinary destruction. No output/filesystem effects occur here.

## Preserved option behavior

The recognizer contains one row per exact spelling, with explicit command
membership and arity. `--root`, `--standard-root` and `--exclude` apply to
check/status/native/emit;
output, SDK, compiler and C flags apply to native/emit. Protocols and creation
apply only to native, compilation only to emit, and overwrite to native/emit.
Neither prefixes nor `--name=value` are inferred. Values are exactly the next
argument, even when beginning with `-`; this parser does not reinterpret shell
quoting or infer a missing value from its spelling.

`--root` records project/import authority; `--standard-root` records standard
library authority, as demonstrated by the [canonical integer CLI](scalar-library.md).
No physical path spelling selects a root kind.

Repeated roots of either kind, exclusions and C flags append in argument order, including
duplicates. Repeated output/SDK/compiler/protocol values keep the last value.
Boolean flags are idempotent. These are the existing behavior preserved by the
refactoring, not a newly selected duplicate-option policy. Native overwrite or
protocol selection requires creation; native/emit require output and SDK. The
existing compiler default is `gcc`. Protocol contents remain the native
command's responsibility, rather than a second ABI parser here.

Missing operands or an unsupported logical path fail before allocation. A final
unknown/inapplicable spelling reports the existing missing-value diagnostic;
with a following argument it reports unknown option. This precedence is preserved
explicitly rather than changed during extraction. Successfully recognized flags
consume no value. Success attaches the same exclusion array to every root and
marks the owner valid only after all relations pass.

## Invariants and refinement argument

Let N be argc. Every root after the first, exclusion or C flag consumes its own
option/value pair in argv[4..N). Hence each of the three arrays has at most N
entries, and allocating N entries is sufficient without a guessed limit. Check
`N * sizeof(ModuleRoot)` and `N * sizeof(char *)` representability before allocating.
The arena checks its allocation-header overhead separately. Failed allocations
remain owned and every complete/partial prefix is released exactly once.

At each consumption step, the index points to the next unconsumed option; its
successful transition advances by one for a flag or two for a valued option.
It never reads a value before proving it exists. Installed arrays preserve the
corresponding argv subsequences, scalars equal their latest supplied value, and
flags equal the disjunction of their recognized occurrences. These invariants
establish the final configuration by induction over consumed options.

Each row in the recognition table denotes exactly one branch of the prior
string-equality chain, with the same request predicate. Installation destinations,
operand checks, native/emission relations and publication are unchanged. Thus
separating these phases preserves success configurations, borrowed identities,
diagnostic precedence and cleanup behavior over the complete valid argv domain.
Tests independently enumerate the fixed 4-by-10 applicability/arity relation,
not a copied implementation or a second legacy parser.

The index strictly increases within finite argv, so parsing terminates. The
spelling table is fixed: time is O(total argument bytes + N), owned array storage
O(N), and other state O(1), excluding the delegated module-path validation cost.
No source tree is loaded, and no native callback or C toolchain runs.

## Evidence

`make -C tests/unit/driver/options` independently exercises the real children and
orchestrator through five C/assert cases: exact recognition across four requests;
repeats/borrowed values/shared exclusions; invalid/missing/inapplicable options;
native creation/check defaults; and every allocation-failure prefix plus explicit
unrepresentable storage. The allocator checks balanced live allocations. Strict
release flags include `-Werror`, `-DNDEBUG` and the leaf's final `-UNDEBUG`.

`make -C tests/unit/driver` connects request/options/run leaves through the real
parent. Real CLI compositions are `tests/integration/check`, `scaffold` and
`execution`; they check check-only effects, native authoring and compiled project
publication. Build the shared compiler before running independent integration
leaves concurrently: they invoke it and must not overlap its relinking.
The root `make -j4 test` enforces this dependency before entering the test tree.

The current ready/debt registry maps each extracted file to named tests and
independently compiles every new/public header. Doxygen verifies each source
page's canonical rule links. Existing source/grammar/context gaps remain the
feature tasks identified in the audit; this refactoring certifies no new language
syntax or runtime service.

T138 acceptance passed the independent strict leaf, the real driver parent,
check/native/emit CLI leaves, standalone header checks and the complete root
`make -j4 test` with 60 registered leaves. ASan/UBSan passed the option leaf with
`detect_leaks=0`; no LeakSanitizer certification is claimed. The initial independent
check-CLI run overlapped concurrent root relinking and failed; after the shared
build completed it passed, and the correctly ordered complete root run passed.
Doxygen verified 448 maintained code/include file pages and all canonical links;
the ready registry now checks 35 inputs with 567 explicit remaining debts.
