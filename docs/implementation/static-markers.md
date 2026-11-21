# Static marker Types

## Task and authority

[T221](../../DONE.md#t221) implements
[TPL-14](../../spec/language/static-results.md#tpl-14). T220 specifies the
notation; T231 and T232 supply canonical identities, retained configuration
and exact dependency provenance. No specification amendment is needed.

## Plan and current step

1. Inventory parser activation, dependency roles, nominal terms, static results,
   ordinary Type resolution, executable/library admission and editor consumers.
   Identify the exact boundary separating a marker from a nonempty producer.
2. Classify a successfully parsed empty static body as a dedicated source kind.
   Lexical/section/body validation must finish first: invalid input is never
   classified by absence of successfully collected statements.
3. Carry marker paths through existing closed nominal Type interning and named
   static equations. Preserve original module identity and zero captures; reject
   producer invocation, export selection and application inputs.
4. Refuse runtime storage at original definition and final substituted Type
   boundaries, and reject executable/native/library roots. Retain source-only
   editor navigation without inventing a runtime Type.
5. Test positive forwarding/identity, distinct and standard identities, ordinary
   producer regressions, all storage forms, malformed sources and every relevant
   measured allocation failure. Run real parents, sanitizers, build/documentation
   gates; record current conformance refusal without renewing T237 fingerprints.

## Representation and correctness obligations

The parser owns classification and immutable syntax. A marker has no graph
statements and no declarations. The inventory retains the exact loaded module
owner, and nominal term interning compares that owner plus its empty capture map.
No runtime slot or payload is created. Dependency roles distinguish a Type head
from a producer statement or selected export; namespace use cannot supply an
implicit marker output. Ordinary admission must inspect the resolved nominal
origin after substitution, not merely its spelling or initial placeholder.

All classification scans and original graphs are finite. Existing exact nominal
keys and equation algorithms supply identity and termination; marker support
adds no recursive expansion or new inference variables. Allocation and failure
ownership remain with each existing arena, and public outputs change only after
complete success. Actual costs and acceptance partitions will be recorded with
the affected consumers.

## Investigation checkpoint

2026-11-21, after T232 commit `b77393c`: the parser currently accepts an empty
static graph but retains MODULE_STATIC, causing all uses to be treated as
producers. Nominal construction already admits zero-input nongraph definitions.
A dedicated marker source kind can therefore reuse nominal identity rather
than adding a parallel Type representation. The runtime/publication and root
admission consumers require explicit checks before this representation is safe.
No T221 implementation or acceptance is claimed at this checkpoint.

## Implementation step and observed results

The complete parser now publishes MODULE_MARKER only after successful empty
graph parsing. Dependency roles admit it as a Type head but reject constant
namespace, producer and export-selector uses. Existing nominal term interning
and Type equations carry the original zero-capture identity, including package
and standard-root distinctions. Alias and named-input/output forwarding use
the same existing terms; no second representation or nominal name is generated.

Known direct marker declarations fail during inventory checking, including
uninstantiated definitions. Resolved ordinary exports, namespaces and actual
field validation reject marker origins after substitution. Compiler roots,
runtime binders/scopes, closure collection and editor runtime metadata exclude
markers. Static value evaluation reuses the closed identity without constructing
an actual marker context. This last boundary was found by the first real graph
test: ordinary creation had left an uncheckable runtime candidate. The exact
closed-marker reuse rule fixes that classification; tests assert no marker
context or SemanticType is created. Initial refusal is retained in
`/tmp/ama-t221-markers-first.log` and `...-diagnostic.log`; corrected forwarding
and all measured frontend allocation failures pass in
`/tmp/ama-t221-marker-forwarding.log` and `...-boundaries-final.log`.

Project role tests include all five source kinds and complete requester
provenance (`/tmp/ama-t221-project-roles.log`, exit 0). Parser classification
checks marker/draft/producer boundaries, comments, CRLF/virtual final newline,
malformed indentation/encoding/sections and failed partial graphs
(`/tmp/ama-t221-parser-classification-final.log`, exit 0). Library resolution
rejects marker entry/read exports (`/tmp/ama-t221-library-roots.log`, exit 0).

The parser parent reaches the known pending T226 canonical constructor example
(language/templates.md fence 3) and refuses its unsupported declaration form;
this is retained at `/tmp/ama-t221-parser-parent.log`, not reported as a parent
pass. `make conformance-check` refuses stale specification evidence as expected
(`/tmp/ama-t221-conformance-stale.log`); T237 owns renewal. No fingerprint was
changed. The compiler parent found a source-delivery fixture missing its now
required explicit external-version selector. Its project package now declares
that selector; source edge/cache expectations remain unchanged. The failed
parent log is `/tmp/ama-t221-compiler-parent.log`; the isolated corrected
fixture run is `/tmp/ama-t221-delivery-selected.log`.

Current jobs: static parent session 29580, editor parent 35455, corrected
source-delivery 4679, and verified generated marker integration  with log
`/tmp/ama-t221-generated-marker.log` (collect its session before acceptance).
T221 remains active; per-file evidence, sanitizers and final documentation gates
are still required.

## Exact admission argument and costs

The marker classification transition inspects only the successfully parsed graph
head. It is O(1), adds no allocation, and occurs after all physical/layout and
section/body validation. A syntax error returns before that transition, so an
invalid partial parse cannot be mistaken for an empty file. Marker parsing does
not skip the lexical rules or classify drafts by a filename.

A marker nominal has zero formals and zero captures. The existing canonical
origin pointer represents one complete logical identity; nominal interning is
therefore exact equality of that origin and the empty map. Distinct modules or
root families cannot merge. Existing term/category checks reject input
applications of a Type. Dependency-role checks reject producer statements and
export selection before those interpretations are evaluated. Static alias and
named-output equality uses the existing solver and changes no nominal key.

Creation classification returns REUSE for a normalized marker nominal, as it has
no body or instance to validate. This removes a nonexistent creation obligation;
it does not mark an unchecked body complete or weaken a recursion guard for an
ordinary module or nonempty producer. Tests inspect the complete context list
and runtime SemanticType list, and independently call the public runtime binder
and typed-scope APIs to check their own refusal boundaries.

Runtime storage checks use the nominal origin, not the displayed source name.
A direct marker root in an ordinary declaration is rejected during definition
inventory; selected ordinary exports are rejected before consumer equality;
resolved field/namespace checks cover actual substitution. The four field forms
share this Type check, and all four are independently exercised. Source-only
static graph outputs remain valid marker Types. Executable/library roots use
the original module classification and never reach instance creation. Editor
navigation retains the original source module and an absent runtime Type.

All added admission decisions are constant-time enum/pointer checks after their
existing finite lookup/normalization phases. Nominal lookup retains its existing
linear scan in the finite term store; no additional index or search is introduced.
Marker support adds no runtime memory, recursion expansion, retries, source
mutation, filesystem dependency or ownership protocol. Failed preparation uses
the existing arena teardown; the complete frontend test refuses every measured
allocation and asserts counted ownership returns to zero after destruction.

## Acceptance continuation

The corrected compiler parent passed (session 20051,
`/tmp/ama-t221-compiler-parent-current.log`). Editor parent passed (35455,
`/tmp/ama-t221-editor-parent.log`). The generated marker program passed actual
C compilation/execution (72425, `/tmp/ama-t221-generated-marker.log`), with the
same exact integer output. The first full documentation gate passed (24011,
`/tmp/ama-t221-documentation.log`): 3248 ready inputs, zero debts, 2447 maintained
file pages, 9425 documented interfaces and all 489 referenced destinations.
Additional direct runtime API and real editor marker-navigation tests were then
added; their final focused/sanitizer and documentation acceptance is still
pending. The static parent remains active as session 29580; collect it before
archival. New jobs: editor marker leaf 7833; marker ASan/UBSan leaf 76153.

The additional marker editor leaf (7833,
`/tmp/ama-t221-editor-marker.log`) and complete marker sanitizer leaf (76153,
`/tmp/ama-t221-markers-sanitized.log`) both exited zero. The sanitizer invocation
uses `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`,
`UBSAN_OPTIONS=halt_on_error=1`, C flags `-O0 -g -Wall -Wextra -std=c11 -pedantic
-D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer
-fno-pie` and `LINKER_FLAGS=-no-pie`. This is ASan/UBSan evidence, not an LSan
claim; counted teardown independently covers every measured refusal. No maintained
production code changed after the first full documentation gate. Final complete
metadata/interface/rendered checks use `/tmp/ama-t221-documentation-final.log`.

Final complete documentation gate exited zero (30280,
`/tmp/ama-t221-documentation-final.log`): 3248 ready inputs, zero explicit
debts, 2447 maintained file pages, 9427 documented interfaces and all 489
referenced destinations validated. Its accepted fingerprints are retained in
`/tmp/ama-t221-accepted-manifest.json`. Source and registry input bytes must
remain unchanged until final archival checks. The only active acceptance run
is static parent session 29580, log `/tmp/ama-t221-static-parent.log`; it has
reached the registry identity tests without errors. Do not archive or commit
T221 before collecting its exit. After a successful exit, archive the full
record, regenerate documentation for narrative-only changes, verify accepted
implementation and final staged fingerprints, and commit T221 independently.

A final coverage review adds an independent ordinary-namespace API case:
`it_refuses_substituted_marker_namespaces_without_publishing_a_value`. It binds
a generic original Type variable to the exact marker term, then checks refusal
and unchanged output at `static_ordinary_resolve`, before runtime field validation
is requested. This tests that public boundary itself rather than relying only
on an earlier compiler rejection. Production code is unchanged. The new case
will run after static-parent session 29580 completes, avoiding overlapping a
recursive suite with its leaf. Per-file metadata and final documentation must
then be accepted again for this changed test input; the previously saved task
input fingerprints deliberately precede it.

The full static parent exited zero (29580,
`/tmp/ama-t221-static-parent.log`), executing 93 assertion harnesses across all
registered static component/composition suites with no reported warning or
failure. The independently added namespace-resolution case then passed with
its entire ordinary-publication leaf (93391,
`/tmp/ama-t221-namespace-boundary.log`). It asserts the specific marker conflict
and unchanged output, not merely a generic unsuccessful frontend result.
Final documentation acceptance for the extended registry/case uses
`/tmp/ama-t221-documentation-namespace-final.log`. After it passes, only narrative
archival/status links may change; verify final source/test/registry fingerprints
before the task commit. No unresolved semantic question was discovered.

## Final task acceptance

T221 is complete on 2026-11-21; earlier continuation checkpoints are historical.
All behavior checks above have collected successful exits. The complete final
documentation gate (8055, `/tmp/ama-t221-documentation-namespace-final.log`)
passed 3248 ready inputs, zero debts, 2447 maintained file pages, 9428 documented
interfaces and all 489 referenced destinations. Final accepted snapshots are
`/tmp/ama-t221-accepted-manifest.json` and
`/tmp/ama-t221-accepted-task-inputs.json`; the latter includes maintained fixture,
Make and coverage metadata inputs in addition to rendered source files.

The full record is [DONE T221](../../DONE.md#t221). T226 is the next ready task.
Only narrative status/archival links change after this acceptance. Supported
Doxygen regeneration at `/tmp/ama-t221-documentation-archived.log`, agreement
of final report/site manifests, unchanged accepted non-Markdown inputs and exact
staged-file hashes are required before the independent T221 commit. No source
or registry edit, semantic question, active behavior test or unfinished
documentation-debt owner remains.
