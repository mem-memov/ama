# Required project dependency closure

The boundary in [project.h](../../src/project/dependencies/project.h) implements
[DEP-01–DEP-03](../../spec/implementation/compiler.md#dep-01) and the source-path
rules in [SRC-01–SRC-05](../../spec/language/source-and-names.md#src-01). It loads
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
fixed `..integer` interface type at its actual target span; no synthetic source
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
