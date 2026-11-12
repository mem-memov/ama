# Retained library, fixture and build contract

T151 closes file-level documentation and executable-consumer evidence for the
retained library, tests and Make infrastructure. Doxygen generates the C file,
type and function reference from comments with `make docs`; this chapter explains
fixture ownership and the exact input exceptions. It is not a second language
specification. The [roadmap](../../TODO.md) and
[library boundary](../../stdlib/README.md) state the missing language features.

## Retained scalar library

The physical `stdlib/modules/std/` sources still use the audited older root and
source forms. Their acceptance does not implement the canonical `..integer`,
condition-capable failure module, current standard-root resolution or typed list
services. Those remain T016, T085, T086 and the list tasks.

The retained index owns one size_t value. Decimal literal parsing checks the next
multiply/add bound before mutation, rejecting empty/nondigit/overflow input.
Condition additionally rejects values above one. Default is zero, clone copies
native data, and destruction owns no heap resource. Explicit condition/index
protocols return the stored value under their declared conversion contract.
Arithmetic modules read ordered inputs, precheck successor/addition overflow and
write their sole result only on success; comparison produces a binary condition.
The status payload copies an immutable program-lifetime message view, not owned
message bytes. Refusal reads the immediate caller snapshot and never writes its
caller or retains a service pointer. All entry input/output/native service views
are operation-scoped. Generated naming contexts qualify authored local names;
persistent Ama references never hide in native payloads.

These algorithms terminate on finite literal bytes and use constant auxiliary
space. Checked decimal accumulation maintains the exact already-consumed prefix
value; the precheck is algebraically equivalent to the next value being at most
SIZE_MAX. Arithmetic prechecks use subtraction rather than overflowing the sum.
Real generated-library execution tests compose these payloads with lifecycle,
interface defaults and named/textual binding order. Native ABI and generated-entry
signature tests separately check the C boundary; no source-shape guess substitutes
for compilation and execution.

## Fixture ownership and assertions

Each registered leaf has one executable main and an explicit run of named `it_*`
assertion cases. Shared guarded common.c files are included by that executable,
not linked as unrelated global owners. File purpose comments identify the fixture
boundary; meaningful helper/type comments state owned resources and observations.
The ready registry maps each maintained file to actual named consuming cases,
including shared support and native companions. A test executable is checked by
running its assertions; it does not need a duplicate suite that mirrors its code.

Counted allocators fail exactly at ordinal N and track successful live allocations.
They permit exhaustive failure-prefix checks of the actual operation, without
random sampling or guessed allocation budgets. Source/project/semantic/plan and
scaffold owners are destroyed in reverse borrow order. Native test resources record
self-addresses and constructor/destructor sequences, detecting relocation,
double cleanup, missed prefixes and primary-error replacement. Metadata-only
fixtures are not claimed to implement a scheduler or independent context API.

Process fixtures use explicit argv, fork/exec and checked normal exit; capture
streams and copied filesystem projects remain owned by the parent case. Expected
compile/runtime rejection must be asserted explicitly rather than inferred from
any child failure. Production object verification is exercised on actual compiler
output. Recursive test-harness and incremental-build tests copy the maintained
Make rules into isolated generated projects, leaving repository inputs untouched.
Successful leaves clean generated output; failures preserve it for diagnosis.

## Header checks and authored fragments

Ordinary C headers compile independently under strict C11. An authored native
payload .h is a fragment included by a generated independent interface: its domain
explicitly includes the SDK types and a caller-established AMA_NATIVE namespace.
The standalone check supplies exactly that documented context through stddef and
one isolated macro definition, then compiles the real fragment. It does not supply
missing field types or substitute a synthetic payload. Actual native/interface
consumers additionally compile the generated declaration context.

`tests/integration/check/project/value.h` intentionally contains #error. Its purpose
is to prove check-only never invokes C, so successful C compilation would violate
its fixture contract. The coverage checker independently compiles this real header
and requires the configured exit code 1 and its exact #error diagnostic. Only an
explicit test-project header may register that negative contract. Wrong diagnostic,
unexpected acceptance, malformed contract or tool exit 127 fails the documentation
check. This is rejection evidence, not an omitted header check.

## Build boundaries and exact invalidation

The sorted recursive source enumerator accepts finite ordinary directories and
returns every nested C path. It does not inspect generated output or use a source
count/depth heuristic. Test source lists compose the exact production stage under
test. Parent Makefiles dispatch explicit independent subdirectories; leaf rules
compile local and production objects separately, emit dependency files, track full
compile/link commands and force assertions with -UNDEBUG even under -DNDEBUG.
Configuration comparison uses isolated candidate files and propagates errors;
unchanged configurations preserve timestamp identity.

Incremental fixtures observe exact compile/link records and the resulting program
value. They distinguish nested header dependencies, unrelated objects, changed
flags, changing source sets and concurrent same-configuration manifest checks.
Harness fixtures prove setup/run/teardown ordering and preservation after failure.
Filesystem/process termination depends on the supported host; no language timeout
or approximate invalidation rule is introduced. Technical reference output remains
Doxygen-owned under docs/technical and is never assembled by handwritten API pages.

## Byte-preserved declarations and expected observations

The following maintained non-C inputs keep their exact pre-T151 bytes. Adding
comments would shift diagnostic byte spans, change literal/draft cases or alter
explicit ABI records. Their exact purpose is recorded here instead. Some are
accepted older-source fixtures, not current-language conformance claims.

| Exact path | Purpose and consuming observation |
|---|---|
| `stdlib/modules/std/condition` | Ordered constants/interface declarations of the retained scalar native module; generated library tests consume its original source form. |
| `stdlib/modules/std/condition.abi` | Exact explicit lifecycle/conversion binding records for the paired retained native module. |
| `stdlib/modules/std/index` | Ordered constants/interface declarations of the retained scalar native module; generated library tests consume its original source form. |
| `stdlib/modules/std/index+/add` | Ordered constants/interface declarations of the retained scalar native module; generated library tests consume its original source form. |
| `stdlib/modules/std/index+/add.abi` | Exact explicit lifecycle/conversion binding records for the paired retained native module. |
| `stdlib/modules/std/index+/before` | Ordered constants/interface declarations of the retained scalar native module; generated library tests consume its original source form. |
| `stdlib/modules/std/index+/before.abi` | Exact explicit lifecycle/conversion binding records for the paired retained native module. |
| `stdlib/modules/std/index+/next` | Ordered constants/interface declarations of the retained scalar native module; generated library tests consume its original source form. |
| `stdlib/modules/std/index+/next.abi` | Exact explicit lifecycle/conversion binding records for the paired retained native module. |
| `stdlib/modules/std/index.abi` | Exact explicit lifecycle/conversion binding records for the paired retained native module. |
| `stdlib/modules/std/refusal` | Ordered constants/interface declarations of the retained scalar native module; generated library tests consume its original source form. |
| `stdlib/modules/std/refusal.abi` | Exact explicit lifecycle/conversion binding records for the paired retained native module. |
| `stdlib/modules/std/status` | Ordered constants/interface declarations of the retained scalar native module; generated library tests consume its original source form. |
| `stdlib/modules/std/status.abi` | Exact explicit lifecycle/conversion binding records for the paired retained native module. |
| `tests/integration/check/project/draft` | Deliberately inactive draft; companion files or embedded selector-looking data must not activate it. |
| `tests/integration/check/project/main` | Explicit root declarations/body selected by this leaf; diagnostic line/byte positions remain fixed. |
| `tests/integration/check/project/value` | Exact Unicode logical module identity and activation input; paired catalog/semantic assertions observe this original path and nominal type. |
| `tests/integration/check/project/value.abi` | Exact explicit native protocol records consumed by the paired ABI/semantic fixture; equal local names must retain distinct module identity. |
| `tests/integration/execution/project/broken` | Deliberate structural or native-build rejection input selected by this leaf. |
| `tests/integration/execution/project/broken.abi` | Exact explicit native protocol records consumed by the paired ABI/semantic fixture; equal local names must retain distinct module identity. |
| `tests/integration/execution/project/main` | Explicit root declarations/body selected by this leaf; diagnostic line/byte positions remain fixed. |
| `tests/integration/parser/grammar/expected-abi-error.txt` | Exact duplicate-ABI-record diagnostic including prior binding span. |
| `tests/integration/parser/grammar/expected-error.txt` | Exact rejected-input diagnostic, source bytes, spans and related context for this leaf. |
| `tests/integration/parser/grammar/expected.txt` | Exact successful parser/source diagnostic observation including preserved literal bytes. |
| `tests/integration/parser/grammar/project/bad.abi` | Exact explicit native protocol records consumed by the paired ABI/semantic fixture; equal local names must retain distinct module identity. |
| `tests/integration/parser/grammar/project/broken` | Deliberate structural or native-build rejection input selected by this leaf. |
| `tests/integration/parser/grammar/project/draft` | Deliberately inactive draft; companion files or embedded selector-looking data must not activate it. |
| `tests/integration/parser/grammar/project/native` | Exact Unicode logical module identity and activation input; paired catalog/semantic assertions observe this original path and nominal type. |
| `tests/integration/parser/grammar/project/native.abi` | Exact explicit native protocol records consumed by the paired ABI/semantic fixture; equal local names must retain distinct module identity. |
| `tests/integration/parser/grammar/project/other` | Exact Unicode logical module identity and activation input; paired catalog/semantic assertions observe this original path and nominal type. |
| `tests/integration/parser/grammar/project/other.abi` | Exact explicit native protocol records consumed by the paired ABI/semantic fixture; equal local names must retain distinct module identity. |
| `tests/integration/parser/grammar/project/程序` | Byte-exact compound grammar input with Unicode names, literal TAB/CRLF bytes, bindings and control forms. |
| `tests/integration/project/closure/expected-error.txt` | Exact rejected-input diagnostic, source bytes, spans and related context for this leaf. |
| `tests/integration/project/closure/project/main` | Explicit root declarations/body selected by this leaf; diagnostic line/byte positions remain fixed. |
| `tests/integration/project/closure/project/中间` | Intermediate requester whose missing dependency must retain the complete diagnostic chain. |
| `tests/integration/semantic/types/expected-error.txt` | Exact rejected-input diagnostic, source bytes, spans and related context for this leaf. |
| `tests/integration/semantic/types/project/main` | Explicit root declarations/body selected by this leaf; diagnostic line/byte positions remain fixed. |
| `tests/integration/semantic/types/project/元` | Exact Unicode logical module identity and activation input; paired catalog/semantic assertions observe this original path and nominal type. |
| `tests/integration/semantic/types/project/元.abi` | Exact explicit native protocol records consumed by the paired ABI/semantic fixture; equal local names must retain distinct module identity. |
| `tests/integration/semantic/types/project/米` | Exact Unicode logical module identity and activation input; paired catalog/semantic assertions observe this original path and nominal type. |
| `tests/integration/semantic/types/project/米.abi` | Exact explicit native protocol records consumed by the paired ABI/semantic fixture; equal local names must retain distinct module identity. |
| `tests/integration/source/diagnostics/expected-ambiguity.txt` | Exact conflicting-root/module identity diagnostic. |
| `tests/integration/source/diagnostics/expected-error.txt` | Exact rejected-input diagnostic, source bytes, spans and related context for this leaf. |
| `tests/integration/source/diagnostics/expected.txt` | Exact successful parser/source diagnostic observation including preserved literal bytes. |
| `tests/integration/source/diagnostics/project/other/商品` | Exact Unicode logical module identity and activation input; paired catalog/semantic assertions observe this original path and nominal type. |
| `tests/integration/source/diagnostics/project/商店/商品` | Exact Unicode logical module identity and activation input; paired catalog/semantic assertions observe this original path and nominal type. |
| `tests/unit/native/abi/hooks/project/buffer.abi` | Exact explicit native protocol records consumed by the paired ABI/semantic fixture; equal local names must retain distinct module identity. |
