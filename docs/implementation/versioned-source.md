# Versioned source identity and package selection

[Integration task T231](../../TODO.md#t231) and
[foundation task T258](../../DONE.md#t258). Status: T258 foundation accepted;
T231 integration remains pending. No complete compiler/project acceptance is claimed. The user authorized implementation on
2026-11-20 after T243 planning.
The canonical rules are [PKG-01](../../spec/project/packages.md#pkg-01),
[PKG-02](../../spec/project/packages.md#pkg-02),
[SRC-01..05](../../spec/language/source-and-names.md#src-01),
[SYN-06](../../spec/language/source-and-names.md#syn-06) and
[TPL-01](../../spec/language/templates.md#tpl-01). This record does not amend them.

## Planned steps and acceptance

| Step | Purpose / handoff | Verification | State |
|---|---|---|---|
| 1 | Recognize bounded exact version components and canonical identity structure without filesystem access or numeric overflow | Independent version/path positive, negative, boundary and bounded exhaustive cases; standalone headers | T258 foundation tested; T231 consumer migration pending |
| 2 | Select whole package-version copies before module discovery; retain physical plus/source mapping and exact aliases/errors | Real catalog/source snapshot, missing-local-hole, duplicate, Unicode and allocation/I/O checks | T258 selection tested; T231 catalog/submodule integration pending |
| 3 | Recognize slash paths and single-dot atomic static names without lookup; preserve source/literal spans | Lexer/parser components and real ordinary/static/constant consumers; malformed spacing/separators | Planned |
| 4 | Migrate identity/dependency/native/source-tool consumers and retained fixtures; delete obsolete dotted-root routes | Actual composed semantic/codegen/native/editor/relocation tests and compiled execution | Planned |
| 5 | Reconcile correctness/ownership arguments, per-file evidence and documentation; archive and commit | Required focused/parent checks, source links, final documentation provenance | Planned |

## Initial findings

At input commit `2dc75ac`, source catalog paths, identity component splitting,
lexer root tokens and parser static names still implement the superseded dotted
model. Physical plus-directory storage and byte-exact filesystem probing are
existing reusable foundations. A complete package selection must precede their
module-level duplicate checks, rather than becoming module shadowing.

Version numbers are lexical digit strings, not host integer values. Recognition
must therefore accept arbitrarily long finite decimal components without numeric
conversion, while enforcing the exact zero/leading-zero rules. Variant validation
reuses the pinned single-word Unicode predicate. All input views remain borrowed;
recognition allocates nothing and changes no bytes or filesystem state.

The original queue's authorization summary was updated to record the new user
request. T222's specification amendment remains a separate gate. No ambiguity
has been identified in the currently examined version/identity contracts.
The first implementation inventory showed that bounded recognition and physical
version selection have independent public contracts and tests. They are extracted
as T258, with T231 retained as their explicit compiler/catalog/parser integration
owner. This is a task/commit boundary refinement inside the authorized scope,
not a new language rule or a declaration that the remaining T231 work is complete.

## Evidence and continuation

Step 1 kernels are implemented in `src/source/path/version.{c,h}` and
`src/source/path/logical.{c,h}`. The existing catalog/parser still use their old
paths pending the next migration step; these kernels alone do not establish
versioned project support. T258 owns these independently tested components; T231
owns their pending integration with compiler/catalog/parser consumers.

`make -C tests/unit/source/path` passed twice as the version and endpoint
partitions were added. The expanded leaf then passed under ASan/UBSan with
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`,
`UBSAN_OPTIONS=halt_on_error=1` and
`C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer'`.
Exit 0; log `/tmp/ama-t231-path-sanitizers.log`. This is memory/undefined-behavior
instrumentation, not a LeakSanitizer claim. The source parent
`make -C tests/unit/source` also exited 0
(`/tmp/ama-t231-source-parent-step1.log`), exercising the newly registered leaf
through its real parent as well as unchanged source regression consumers.

The leaf checks all 97,656 strings of length 0..7 over `v01.-` against an
independently constructed finite grammar language. At these lengths no variant
fits, and numbers have at most two digits from that alphabet; the grammar's
possible numeric strings are exactly `0`, `1`, `10`, `11`. Independent positive
and negative vectors separately cover Unicode variants, truncated/malformed
encoding, hyphen/space rules and host-range independence (4096 digits per field).
576 constructed package-shape combinations check structural endpoint membership.
Bounded unterminated/NUL views and 1024-component finite namespaces cover byte
bounds and absence of a semantic depth cap. These finite domains are not a claim
to enumerate the infinite accepted language.

Correctness: decimal scans consume precisely a nonempty digit run and reject
multi-digit leading zero; three runs have exactly two intervening dots and only
the optional complete word remains. The endpoint state records vendor, one-or-more
package names, the unique version boundary, src/tests and one-or-more module
names; the standard branch instead requires only complete ordinary names.
Every successful loop advances a bounded cursor. The components and Unicode
word scans cover disjoint spans a constant number of times: O(input bytes)
time and O(1) auxiliary space, no integer conversion/allocation or retained views.

New source/header/test/Makefile evidence is registered by exact paths and ten
named assertion cases in the existing per-file registry. The first
`make docs-check` exited 2 because the two new headers lacked their explicit
standalone compilation commands in that registry. Added those commands using
the existing strict C11 header contract. The corrected `make docs-check` exited
0 with 3034 ready inputs and zero debts
(`/tmp/ama-t231-docs-check-step1-fixed.log`), including both independent header
compilations and actual named-case interface checks.

## Whole-version selection contract and argument

`src/source/module/packages/packages.h` exposes selection for a fresh initialized
catalog, stable borrowed root descriptors and a finite prepared filesystem tree.
It does not claim a filesystem snapshot against unrelated concurrent writers.
It owns no source/parser state and reads no version's modules or configuration.
It selects candidate version directories; validation/loading of the selected
version's required files remains a later consumer's obligation, with no fallback.

The output is a sorted linked list in the catalog arena. All path strings and
nodes remain owned until catalog destruction; each node's root descriptor is
borrowed through the enclosing selection/indexing call. The output pointer
changes only on complete success. Partial queue/selection storage and exact
error context remain owned for inspection/destruction after any failure.
The operation never modifies source files or publishes a partially built module
catalog. It cannot be used on an already discovered/indexed catalog.

Root validation, optional storage discovery, candidate classification, FIFO
ownership and sorted selection publication have separate implementation files.
First process every configured ordinary root's local storage, then every external
storage. For each candidate, the logical key is the full vendor/package/version
prefix. Consequently a local key is already known when an external candidate is
considered; ignore the latter without entering its version contents. Same-priority
duplicates fail rather than merging trees or selecting by traversal order.
Different versions and complete variants remain distinct; sorted reporting is
not a highest-version choice. Standard roots do not contribute ordinary versions.

Induction over queued namespace directories preserves their exact physical,
relative and logical prefixes and vendor/package depth. A version boundary is
recorded, never enqueued; non-directory files outside versions are not module
candidates. Pinned Unicode/name and version predicates decide candidate spelling
without looking up declarations. Physical directory registration rejects aliases
before requeueing, including empty repeated project roots. Every successful scan
consumes a finite entry list, and each namespace directory is queued once, proving
termination without recursive C calls or a semantic depth bound.

Sorted insertion preserves one record per key and publication only after both
passes establishes whole-version precedence independently of root order. Exact
probes/entry enumeration own and close their streams; catalog arenas own all
scratch, so aborting a child leaves no detached allocation owner. A code review
found that storage-path allocation failure could otherwise retain a caller's
root string in the error record. Discovery now uses the already catalog-owned
verified root spelling; allocation-failure tests reject retaining the borrowed
root address. This preserves diagnostic lifetime even when another allocation
cannot succeed.

Let D count registered namespace/root directories, C version candidates, V
selected versions, B retained path/name bytes and L the largest logical-key byte
length. Directory identity checks cost O(D squared); simple sorted selection costs
O(C V L). Add B, the existing entry-sorting cost and exact physical-probe cost
(including their directory reads) for total work. Arena space is O(B + D + V + E)
for E retained enumeration records. Filesystem/tool limits remain external
failures, not invented limits on the source language.

## Selection verification and remaining delivery gates

The independent leaf `make -C tests/unit/source/module/packages` passed its initial
six scenarios (`/tmp/ama-t231-packages-initial.log`), then the expanded descriptor,
physical-failure and I/O partitions (`/tmp/ama-t258-packages-io.log`). The final
owned-error-context implementation also passed strict ASan/UBSan with the same
flags recorded above (`/tmp/ama-t258-packages-owned-context.log`, exit 0).
Earlier successful instrumented runs are retained as historical input boundaries
in `/tmp/ama-t258-packages-sanitizers.log` and
`/tmp/ama-t258-packages-io-sanitizers.log`.

Nine explicit selection cases cover empty/dependencies-only roots, hierarchical
packages, exact sorted version identities, local priority across both root orders,
local and external duplicate refusal, misplaced boundaries, symlink/UTF-8/storage
shape errors, invalid/repeated descriptors and output preservation. Each measured
allocation can fail in turn; every measured opendir/readdir/closedir/lstat call
can independently report EIO. The I/O fixture uses actual directory operations,
counts acquired streams and requires zero live streams on every exit. Simulated
close failure releases its actual resource before reporting the error, making
cleanup observable rather than claiming unspecified close behavior is universal.

`make -C tests/unit/source` passed with both new leaves registered in the real
parent (`/tmp/ama-t258-source-parent.log`, exit 0). It includes existing source
regressions; their old frontend behavior is not evidence of new project syntax.
`make` also exited 0 (`/tmp/ama-t258-build.log`), compiling/linking the actual
compiler with the new nested source files under its default O2/C11 flags. This
is neither a clean whole-sprint release nor generated Ama execution acceptance.

The per-file registry maps all eleven new production source/header files and
their test/Make/IO support to the ten path and nine selection cases. Five new
headers have independent strict C11 compilation commands. Source explains rule
links, ownership and failures; this record is routed through the component
contract registry. Initial `make docs` exited 0 (`/tmp/ama-t258-docs-initial.log`): 3045 ready
inputs, zero debts, 2312 maintained file pages, 8975 documented interfaces and
487/487 canonical destinations. Doxygen 1.11.0, Python 3.14.4 and pinned
markdown-it-py 3.0.0; the worktree provenance check verified all 2559 rendered
input fingerprints before archival.
No specification or reviewed complete-conformance fingerprints have changed.

The structural/link/task check verifies 258 unique IDs, acyclic prerequisite
closure, nineteen original build/release plan rows, all seventeen new maintained
inputs and five standalone-header commands. Its temporary parser was adjusted
to accept existing prerequisite-label capitalization/colon variants; the final
check passes. `make conformance-check` exits 2 with the expected newer-specification
refusal (`/tmp/ama-t258-conformance.log`); no complete witness is renewed.

Continuation: T258 implementation/component/parent/sanitizer and initial
documentation checks are complete. After archival, the final delivery command is
`make docs > /tmp/ama-t258-docs-final.log 2>&1`, with active-job metadata recorded
at `/tmp/ama-t258-active-job.json`. Collect its exit status and require final
source/staged provenance before committing; a lost/unconfirmed job outcome must
be checked or rerun safely, never inferred. T231 resumes actual catalog/parser/
identity/dependency/source-tool migration after that commit. T237 owns the eventual
whole-sprint conformance renewal; no compiler entry support is inferred from these
foundation tests.
