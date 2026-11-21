# Required project dependency closure

The boundary in [project.h](../../src/project/dependencies/project.h) implements
[DEP-01–DEP-03](../../spec/implementation/compiler.md#dep-01) and the source-path
rules in [SRC-01–SRC-06](../../spec/language/source-and-names.md#src-01). It loads
one exact required graph. It does not instantiate objects, validate eager layout
cycles or claim that structurally accepted syntax already has executable support.

## Owners and publication

Initialize a project, load once, inspect, then destroy. Roots/exclusions and entry
strings are borrowed during loading only; the allocator and stable filesystem
outlive the operation. The project owns its catalog, immutable source bytes,
module ASTs, request paths, graph edges and diagnostics. Destroy ASTs before their
source bytes. Successful and failed partial projects use the same owner teardown.

A registry record is inserted as `PROJECT_MODULE_LOADING` before its dependencies
are considered. It becomes `PROJECT_MODULE_READY` after its own syntax and edge
collection finish. It may still refer to another loading record; only project
`valid` certifies the complete closure. Public `project_find` publishes records
only after complete success. Every syntactic occurrence has its own source-mapped
edge, while all occurrences of one identity share one registry record and AST.

A missing required module, required draft, parser failure or native-companion
failure retains the actual source context and a finite requester chain. A
constants-only dependency is legal, but cannot be the executable entry. Entry
spellings and diagnostic arrays are copied before caller storage expires.
Allocation failure reports OOM without further allocation; a trace cannot be
promised when memory for that trace is unavailable. Failed graphs are retained
for diagnostics/destruction, never published as usable output.

## Complete structural discovery

[AST collection](../../src/project/dependencies/discover/ast.c) visits constants,
parameters, results, body statements, selected-operation trees, every operation
handler, folded loop commands, loop/branch bodies and traversal bodies. Explicit
project-owned work frames replace recursive C traversal.

Declaration type paths, qualified constant sources and constant-read namespaces
contribute requests. A structural list-service parent also requires LIST-04's
fixed `//integer` interface type at its actual target span; no synthetic source
file/path span or family fallback is created. Ordinary list storage/copy/traversal
alone does not add this interface dependency. Calls resolve through declared types later; selector names,
local operands, indices, iterator binders, self references and literals do not
invent imports. Ordinary output-action bindings alias arguments and are not
visited as extra source occurrences. A loop's terminal-owner link also aliases
its command tree and is not traversed twice.

[Request records](../../src/project/dependencies/discover/paths.c) retain every
extracted original span. [Stable bottom-up merge passes](../../src/project/dependencies/discover/sort.c)
sort them by physical byte offset without allocation. Registry admission then
follows that exact order. This makes first-requester selection independent of
whether a header, handler or selected operation occupies a particular AST link.
It also prevents later bindings from being discovered before earlier selected
subtrees merely because ordinary bindings share a separate list.

The [registry](../../src/project/dependencies/registry/records.c) compares complete
logical identities byte for byte, including their explicit root marker. Project
and Standard families never fall back to one another. Appending new records
provides a finite FIFO; an edge to a loading/ready record does not reload it.
Repeated explicit seeds share those same records. Parent source directories do
not create dependencies or inherit declarations.

## Defining scopes and normalized operands (T232)

The [configuration owner](project-configuration.md) prepares one package alias
environment per required retained version and a separate local environment for
each activated definition. Parsed documents are temporary; expanded scope data,
selection metadata and normalized operands belong to the project arena, while
their original source spans borrow captured catalog/configuration bytes. Local
aliases do not inherit from a caller or a containing module. A standard module
has no ordinary package owner. Alias declarations alone import no modules.

Each explicit written operand is normalized under its defining scope before
registry admission. Edges retain the entire original request span and the
canonical module identity. The complete operand of a qualified constant is
retained in the AST without its list suffix; only after alias expansion and
component reduction does the project select its final member and preceding
module. The edge owns that normalized namespace/member descriptor. The member
can originate in an alias file or have no source token when inherited from
canonical directory metadata; no SourceText or member span is fabricated.

Explicit endpoint admission applies the same selected-version permissions and
src/tests direction checks to declarations, static constructions and constant
namespaces. Missing actual targets remain missing-module errors even when a
wildcard selects nothing. Standard definitions cannot author ordinary module
dependencies, while caller-supplied ordinary Type/Template arguments retain their
own originating provenance. The actual finite catalog view retains physical
test-directory versus structural `+` ancestry for associated-test coverage.

Static constant binding selects the exact explicit dependency occurrence.
Its normalized member bytes choose an own declaration after independently
closing the namespace. Ordinary semantics consumes the resulting declaration
proof instead of splitting the written text a second time. Editor indexing uses
only genuine source-local member tokens; an alias key is not a variable name.

Captured frontend consumers explicitly supply version metadata and optional
module companions, including captured absence. Their aliases, constant operands
and standard template arguments use the same pipeline without filesystem
fallback. Complete source-operation adapters and their new command semantics
remain T224 after T235; a captured source-only check does not certify those
commands. The base closure bounds below are supplemented by one actual-view
construction, each needed version's configuration/scope/selection preparation,
and each operand's normalization/admission costs, using the component bounds
linked from the configuration contract. No module is reparsed to obtain an
alias expansion or a second guessed constant split.

## Native activation and invalidation

The catalog indexes source identities and validates their source UTF-8 independently
from required module parsing. It leaves companion fields absent initially.
[Required parsing](../../src/project/dependencies/registry/parse.c) first establishes
module activation. Only a required `@C` source selects its `.c`, `.h` and `.abi`
through [explicit catalog loading](../../src/source/module/resolve/native.c).
Ordinary Ama implementations, constants-only namespaces, unused native modules and
drafts do not acquire native companions. Unusable stale companions next to those
sources cannot become native requirements.

Native selection derives exact physical companion paths from the source filename,
including structural `+` directories. Present companions must be supported regular
files; ABI bytes receive strict UTF-8 validation. Complete companion views publish
atomically after all three probes succeed. A repeated successful selection returns
the same views without reallocation. Partial paths/ABI bytes remain catalog-owned
on failure, but no partial companion set is published. Native errors preserve
physical locations and source-requester notes.

Missing companions remain null at this loading boundary so explicit native
authoring can create them. Strict native checking later requires the companions
and validates ABI records/signatures. The source layer does not parse bodies or
guess activation from C filenames.

[Exact input capture](project-inputs.md) attaches only contributing source/native
inputs and all resolved module edges. A new project owner observes source changes
rather than reusing failed draft records. Changing a required draft to an active
source allows loading; changing `@C` to Ama removes native input edges and
invalidates the module and its consumers. Native file changes affect their actual
consumers, while unused/stale companion files are absent from the graph.

## Laws and correctness

For finite sources, let `R(m)` be the sequence of dependency requests extracted
from module `m` and sorted by their exact source positions. For the explicit entry
and additional required seeds `S0`, the required closure is the least fixed point
of `F(S) = S0 union union(R(m) for m in S)`.

1. Registry insertion checks complete identities first, so induction gives one
   record per identity through arbitrary cycles, diamonds and repeated seeds.
2. Every non-seed record is required by an actual request of a registered module.
   Registration is therefore sound for the least fixed point.
3. Every AST node has one owning traversal edge. Advancing a frame visits its
   next node once; only owned children are queued. Alias links are excluded.
   Induction over the finite structural tree proves complete path extraction.
4. Each merge pass partitions the request list into sorted runs. Merging adjacent
   width-bounded runs preserves their multiset and stable equal-key order while
   doubling the sorted-run bound. Width growth stops before overflow once one
   run covers the list. Thus admission follows the exact physical request order.
5. The FIFO processes every registered module once and adds every request before
   marking it ready. Successful exhaustion establishes closedness; combined with
   soundness, this proves equality with the least fixed point.
6. Existing identities do not append records. Finite module cycles therefore
   terminate without recursive loading or repeated parsing. Layout, constant and
   execution cycles remain separate semantic/runtime checks.
7. Each first requester predates its target's insertion. Following requester
   links strictly decreases insertion rank, giving a finite source-backed trace
   even when ordinary graph edges form cycles.
8. Every AST/native scratch allocation has an owner before fallible work. Native
   views publish only after complete selection. Every failure therefore leaves
   safely destructible state without partial public closure success.

For `V` required modules, `C` catalog records, `E` requests, `N` visited AST
nodes/initializers/arguments, maximum identity byte length `L` and total path bytes
`B`, work after catalog construction is
`O(N + E log E + (E + |S0|) V L + V C L + B)`, plus parsing and explicit native
filesystem probes. The deliberately simple registry/catalog indexes are exact
linear searches. Storage is `O(V + E + N + B)` plus AST/source/catalog ownership.
The traversal, sort and closure algorithms impose no hidden source-depth limit.

## Evidence

`make -C tests/unit/project/closure` checks every directed graph on three modules
against independent reachability, self/mutual cycles, diamonds, repeated seeds,
complete multiword identities in both root families, required drafts, missing
sources and exact requester chains. Expanded scenarios cover selected modes,
handlers, folded headers and traversal bodies in physical order; selector names
do not become imports. All 5040 seven-request permutations are sorted with
monotonicity, multiset and equal-key stability checks, without allocation.

Activation scenarios cover ignored malformed stale companions, required native
UTF-8 failures, draft-to-Ama/native transitions and native-to-Ama input invalidation.
Every actual allocation in expanded successful and native-error closures is failed
once; all counted owners return to zero. The source catalog leaf separately checks
native selection memoization, foreign/unbuilt inputs, exact companion paths and
every native-selection allocation failure with atomic publication. Deep body and
module-chain cases retain explicit frames. Named per-file evidence is registered
in `documentation/ready.json`.

## Static expression dependencies (T171)

Input f6f80c0, clean tree. T170 supplies immutable source-owned expressions in
declaration types, constant namespaces, static result definitions and standalone
projections. The old collector reads only each ordinary declaration/namespace
head; it can mistake a placeholder for a filesystem path and misses concrete
nested operands. This task extends discovery, not static semantic resolution.

Contract: traverse one optional structural expression exactly once per owning
edge. Emit each MODULE_PATH head with its original span. Emit no path for a
PLACEHOLDER head, formal/signature name or output receiver; nevertheless visit
all nested input expressions below a placeholder. Preserve duplicate written
occurrences for the existing stable source-position sort and shared source
registry. No static substitution, scope guess, fallback root, synthetic file or
virtual specialization is created. Source/AST views remain borrowed; work and
request records belong to the project arena. Failure leaves no public valid
closure and uses the ordinary counted teardown.

Algorithm: keep an explicit pending-expression chain, pop one head and push its
nonempty owned input children. Each pop processes a unique finite AST owner;
no output alias supplies another ownership edge. Remaining unvisited owner edges
strictly decrease when consumed, so traversal terminates without C recursion.
For e expression/binding records and p concrete heads the new traversal costs
O(e + p) plus copying/sorting/admission already described above, and O(e + p)
project arena storage, including frames retained until owner destruction.
Adding this complete request extraction preserves the existing closure's least
fixed-point proof; source cycles do not become specialization or layout errors.

Steps: (1) independent expression component tests for placeholder/concrete/nested
heads and exact paths; (2) compose all four AST expression positions;
(3) real two-root/name+/source-cycle/diagnostic/native/failure fixtures;
(4) per-file evidence, whole-consumer regressions, documentation and task commit.
Current step 1; no active jobs or unresolved semantic questions.

Step 1/2 results: `project_collect_expression` is a separate iterative component.
A direct independently assembled tree verifies placeholder roots with concrete
nested inputs, repeated equal spellings with distinct source spans, output aliases,
input skips and allocation-free null input. The AST collector now calls it from
declaration types, constant namespaces, static result expressions and standalone
projections; signature/formal names remain non-filesystem syntax.

Step 3 results: the real closure fixture passes both root orders, exact hyphenated
Project/Standard names, nested `shelf+/child` without loading its draft parent,
explicit static result/projection/alias-namespace forms, a source cycle, shared
source ASTs and required native companions. Each written path retains physical
source order and its original requesting span. Missing/draft/invalid nested
arguments preserve the exact two-module requester chain and do not fall back from
Standard to a same-named Project module. Success, missing and invalid-native
paths pass every actual allocation-failure prefix with zero live counted owners.
A real 768-level static expression produces 769 requests to one loaded definition.

Commands already passed: `make -C tests/unit/project/closure`, logs
`/tmp/ama-t171-expression.log`, `/tmp/ama-t171-closure.log` and
`/tmp/ama-t171-composed.log`. The first new draft fixture used `unfinished [`;
SYN-06 correctly rejected its illegal spacing before draft classification.
Replacing it with lexically valid `unfinished module` exercises the intended
required-draft partition; the corrected composed run passes. No requirement or
expected behavior changed to fit the implementation. New files and changed
interfaces have named per-file evidence in the registry.

Current step 4: settled sources ready for broader project/source/frontend/editor
regressions, strict sanitizers and final documentation acceptance. No semantic
ambiguity or spec change is involved; no specialization identity is allocated here.

Broader acceptance checkpoint: `make`, the complete project parent and source
catalog leaf pass, including existing duplicate/physical-alias/root-family
checks (`/tmp/ama-t171-regressions.json`). The T170 concrete-boundary frontend
test now exposes an incomplete fixture: its standalone `.factory` expression
becomes an actual source dependency under T171, but that scenario never created
the definition. Add that source to the fixture before asserting the later semantic
admission failure; keep the loader's new dependency behavior unchanged. Collect
running editor/integration batch session 90561 and isolated closure ASan/UBSan
before source edits, then rerun the affected compiler leaf. Sanitizer log:
`/tmp/ama-t171-sanitized.log`; leak detection uses the established container limit.

Corrected functional acceptance: the frontend scenario explicitly creates its
`.factory` definition before loading and now reaches the intended concrete-stage
static admission diagnostic; the entire compiler/check leaf passes
(`/tmp/ama-t171-frontend-corrected.log`). The first placement of that fixture
line matched another similar test loop, was caught by the focused check and
moved to the owning scenario. No production behavior or assertion was weakened.

Final broader commands passed: `make`, `make -C tests/unit/project`,
`make -C tests/unit/source/module`, `make -C tests/unit/compiler/check`,
`make -C tests/unit/editor`, `make -C tests/integration/check`. The unchanged
catalog cases cover root duplication, hardlink/symlink/directory aliases and
structural native paths; new compositions exercise the same catalog with actual
static dependencies. No virtual source files are generated or searched.

The complete closure leaf also passes strict ASan/UBSan, including all five new
component/composition cases, existing graph/ordering cases and allocation-failure
prefixes (`/tmp/ama-t171-sanitized.log`):

```sh
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
make -C tests/unit/project/closure \
  C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer'
```

LeakSanitizer has T169's independently reproduced container/ptrace limitation;
counted ownership cleanup remains asserted. Original spans, exact roots, stable
physical request order, shared parsed source definitions and finite source cycles
are established; specialization/layout legality and native template rejection
remain their later semantic tasks. Functional jobs have finished; complete
per-file documentation and Doxygen acceptance are next.

T171 documentation acceptance (2026-11-18): `make docs-complete` checks 2358
ready inputs with zero debt. `make docs` passes all originating-file and actual
rendered destination gates: 1771 maintained file pages, 6779 interfaces, 417
canonical destinations and 5588 pages (Doxygen 1.11.0/Python 3.14.4/markdown-it-py
3.0.0). Deep-source links resolve relative to expressions.c's actual directory;
`code --goto /home/u/repos/ama/spec/implementation/compiler.md:630` returns zero
for DEP-03 in the supported source-navigation setup, without a GUI click claim.
The full task is archived in [DONE](../../DONE.md#t171). Final regeneration checks
archived/staged maintained bytes before the separate task commit; no spec bytes
or reviewed conformance fingerprints changed.

## T213: static path roles

Discovery retains the role of each written occurrence:
ordinary value/namespace, static producer, or explicitly selected static export.
An expression's nested inputs are ordinary values even when its root is a
producer. Placeholder heads, binding labels, receivers and selected export names
never request files. A selected export changes the role of its containing path;
it does not introduce another dependency.

The loader drains its existing finite registry queue before checking these roles.
Every target is therefore parsed when its kind is inspected, including forward
and cyclic requests. A second linear pass over the retained edges rejects the
first incompatible occurrence with its original source span and finite requester
chain. No target is reloaded. The pass takes O(V + E) time and constant auxiliary
storage, apart from existing diagnostic construction on refusal. Failure leaves
`valid` false and all partial allocations owned until normal project teardown.

This check establishes source-kind compatibility only. Export existence,
concreteness, inference, specialization cycles and executable-entry admission
remain with their semantic consumers (T208–T212/T184). A source graph seed is not
an executable instance. Constants-only dependencies retain their existing shared
namespace behavior; this pass does not grant them instance storage or execution.

Acceptance evidence (T213, 2026-11-19):

- The real parser/registry composition covers all 32 combinations of eight
  occurrence positions and four complete kinds: Ama, native, constants-only and
  static. This establishes source roles, not executable validity of the fixture.
- Repeated equal paths retain different roles and exact spans; a nested late
  mismatch preserves both requester locations. Legal and incompatible source
  cycles share registry records; every measured allocation refusal in both
  outcomes unwinds without publishing a partial project.
- Existing closure checks retain both root orders/families, name+/ submodules,
  duplicate/alias rejection, deep trees, missing/draft/syntax errors and native
  companions. Actual selected-export and nested-input capture propagates source
  changes through exactly its static and ordinary consumers.
- `make -C tests/unit/project -j2` passes with strict O0/Werror/C11 and O1
  ASan/UBSan. The final extra occurrence/requester case was rerun in both
  configurations. The same configurations pass native-admission and static
  scope, explicit and classify leaves. The task-only root `make -j4` passes.
  Sanitized commands use `-fno-pie`, `-no-pie`, frame pointers and
  `ASAN_OPTIONS=detect_leaks=0`, the established container restriction;
  counted allocator teardown independently checks fixture ownership.
- During fixture review, the historical success case used a graph as an ordinary
  Type; its loading-cycle assertion was retained with an ordinary self-reference.
  The new nested-error fixture initially failed at an earlier invalid graph
  reference; selecting its named export exposes the intended later occurrence.
  Initial syntax errors in new fixture strings were corrected before acceptance.
- Per-file registrations include all changed production/header/test inputs.
  Final Doxygen generation, rendered rule destinations and task-queue/link
  validation are commit gates; their exact output is retained in the task's
  commit evidence. No complete `make test` or executable-template conformance
  claim follows from these bounded checks.

Reproduction uses `C_COMPILER_FLAGS`, not `CFLAGS`: strict flags are
`-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L`.
For sanitizers replace O0 by O1, append
`-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie`, and pass
`LINKER_FLAGS=-fsanitize=address,undefined -no-pie`. All acceptance runs use an
isolated HEAD-plus-T213 tree, excluding unrelated unfinished T182 changes.
