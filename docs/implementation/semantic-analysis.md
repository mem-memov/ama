# Semantic analysis and resolved plans

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

This describes the semantic subset audited in T001; [TODO](../../TODO.md) tracks adaptation under
NAME-04, TYPE-01/TYPE-05 and COMP-01/COMP-02 of the
[specification](../../spec/README.md). The public boundary is
[semantic.h](../../src/semantic/semantic.h). Passing an earlier stage never
certifies an unimplemented later stage.

[Exact transfers and lexical self](typed-transfers.md) records T032's storage
matrix, constant/indexed source handling and staged executable boundary.

## Namespace construction

`semantic_program_init` borrows a valid loaded project, which must outlive the
semantic owner. Call `semantic_names` once, then inspect only successful namespace
output. Destroying the semantic program releases its ABI tables and arena before
the caller destroys the project. All declaration/use spans borrow that project.
Allocation failure retains an allocation-free diagnostic and partial owned
records solely for cleanup. No C text, native execution or filesystem writes occur.

The [complete declaration boundary](declaration-inventories.md) records T030's
inventory, exact source ordering, shared interface lookup, proof and checks.
[Collection](../../src/semantic/walk/collect.c) visits every owning structural
subtree, including selected handlers, folded headers and traversal bodies.
[Stable event sorting](../../src/semantic/walk/sort.c) establishes physical order
before callbacks; no C recursion represents source nesting. Only loop/traversal
bodies add a loop-depth scope, while header handlers retain incoming context.

[Namespace construction](../../src/semantic/names/build.c) gives every explicit
declaration one complete base-name symbol, module ordinal and role ordinal.
Storage suffixes are attributes, and ordinary blocks do not shadow names.
[Exact lookup](../../src/semantic/names/lookup.c) preserves original UTF-8 bytes;
[interface lookup](../../src/semantic/names/interface.c) shares declared names,
positions, types and storage flags with call checking. The [source-order pass](source-order-inference.md) adds receiver-inferred fields
and classifies foreach-local bindings separately.

The inventory invariant is the unique source-ordered declaration prefix. Exact
comparison either diagnoses the later duplicate with its earlier declaration or
preserves that invariant by insertion. For N nodes, D declarations and maximum
name length L, construction costs O(N log N + D² L) time and O(N + D) storage.
Tests cover all 256 section/storage pairs, expanded source/context ordering,
exact root nominal identities, complete metadata and every allocation failure.
Run `make -C tests/unit/semantic/names`; typed visibility/initializer checks run
through `make -C tests/unit/semantic/types`.

## Native contracts for the required closure

`semantic_native` requires a successful namespace stage and runs once. It visits
only records in the required closure whose parsed selector is `@C`. Missing
`.h`, `.c` or `.abi` companions fail at the selector, naming the module and missing
suffix. ABI tables use the existing fixed-record parser, preserving duplicate
locations and malformed-record errors. Each table belongs to its semantic type:
identical function spellings in different native modules remain different typed
bindings, never global-name lookup. C bodies and signatures are not parsed or
executed; later generated C uses check handwritten signatures.

The complete selection predicate is `required && selector == MODULE_NATIVE`.
Exhaustion of that finite set establishes all required companion/table checks;
unused draft ABI grammar cannot enter the selection. Source catalog checks
apply to indexed module sources; native companions are acquired only after
required parsing establishes `@C`, as described in the [closure contract](project-dependencies.md). The phase costs the sum of required
ABI parsing costs plus `O(V)` type visits and owns those tables until program
destruction. All exits preserve the table/arena ownership established before
parsing. [Native tests](../../tests/unit/semantic/native/main.c) enumerate all eight
presence masks, exercise equal module-local helper names, unused drafts, duplicate
ABI bindings and every native-parse allocation failure. Release/Werror tests pass.

## Resolved values, interfaces and storage compatibility

`semantic_resolve` follows namespace/native validation. The source-order pass first
links explicit value types, then adds exact inferred fields and lexical foreach
bindings before full value checking. Constants-only namespaces cannot be instance
types. It then computes clone capability over all owned
interface/body fields (including list element types), excluding shared constants
and reference edges. Starting with every executable type except native types
without `=`, repeated removal of types owning a removed type computes PROTO-07's
greatest fixed point. No more than `V` types can be removed. This is capability
inference, not an eager-layout cycle test.

[Access resolution](../../src/semantic/values/access.c) matches the full declared
storage form and resolves every named nested index iteratively. Index and condition
conversions require the operand's own native `[]`/`?` binding and an owned scalar.
A reference does not implicitly dereference, nor does a field's capability propagate
to its owner. Mutable access to constants is rejected. Initializer visibility
requires earlier local declarations and restricts shared initializers to constants;
qualified local forward references cannot evade the same ordering.

[Sources](../../src/semantic/values/source.c) retain module identities, original
spans and payload bytes. Qualified sources resolve only exact shared members.
[Transfers](../../src/semantic/values/transfer.c) implement TYPE-05 as a product
of nominal type equality, scalar/list cardinality and ownership form. An indexed
element is scalar storage. A declaration list initializer is checked as one
scalar element, not list splicing. Literal construction selects its destination
owned scalar's `|`; no type or numeric value is inferred from bytes. `%` can clone
an owned/reference scalar into owned scalar storage, subject to clone capability.
Every syntactic owned transfer checks copy capability, including apparent
self-assignment. T147 removes the static no-op enum and spelling comparison:
ordinary self identity is decided at runtime only after normal selections and
access checks. Direct nonindexed `%` self-copy is rejected by exact resolved
symbol identity; indexed/reference aliases retain runtime identity/overlap checks.

[Call resolution](../../src/semantic/calls/resolve.c) binds targets, named labels
or independent positional sequences before consumption. Every placeholder counts
against arity, without a hidden receiver/source. Named labels must match the full
field storage form and cannot repeat in their input/output sequence. Bindings
retain text order and resolved field ordinals separately: installation order is
not confused with evaluation/transfer order. CALL-07 literals derive their type
from the selected parameter, with its declaration retained for diagnostics.
Stored-result reads expose results; namespace reads expose constants; neither
makes body/private fields public. Synthetic output values are typed records,
not fabricated parser nodes or C text.

Whole-list execution and retained-result bindings are rejected before lowering.
The current semantic API accepts no configurable list role types. The target
[LIST-04 interface](../../spec/language/lists.md#list-04) gives each intrinsic
method its own typed inputs/results; T075–T084 implement those interfaces,
method-specific capabilities and service-frame lifetimes. Existing indexed
scalar access and whole-list value transfers remain separate boundaries.
Opaque native constant purity/resource restrictions remain native-library
obligations, never guessed from C helper names or payloads.

The invariants of each stage compose: identity links are complete before any use
check; every resolved access names one symbol and recursively well-typed index;
every successful transfer satisfies exactly one declared storage rule; every call
binding names one legal field/position. Induction over each finite source/access
sequence proves completeness, while nominal pointer inequality rejects distinct
module identities regardless of matching representations. Partial arena results
are never published as `resolved`. With `N` nodes/uses, `D` symbols, `V` types,
`E` owned edges and maximum name length `L`, straightforward lookups/checks cost
`O(N D L + N V L + V E)` plus pairwise named-binding checks; memory is linear in
resolved uses, bindings and traversed blocks. Algorithms do not recurse on source
depth and do not execute native constructors at compile time.

[Type tests](../../tests/unit/semantic/types/main.c) enumerate all 32 combinations
of destination/source storage form and same/different module identity. Additional
cases exercise literal/reference/continuation/default rules, immutable constants,
forward initialization, binding arity/labels/duplicates, retained/static reads,
explicit index/condition capabilities, list mappings/clone restrictions, preserved
payload bytes/order and every resolved-allocation failure. Release/Werror checks
and focused ASan/UBSan supplement the derivation; LeakSanitizer remains disabled.

[T033 relation evidence](type-relations.md) details the independent predicates,
fixed-point proof, admission-order tests and finite failure checks.

## Separate relation graphs and exact ordering

`semantic_relations` requires resolved values and constructs two independent
[dependency graphs](../../src/semantic/relations/graph/graph.h). Vertex keys are
unique full module paths plus declaration ordinals; values/spans remain borrowed
until graph destruction. An edge means its target must be initialized earlier.
The [eager graph](../../src/semantic/relations/eager.c) includes owned scalars and
nonempty initialized owning lists from interfaces/body fields, excluding constants
and references. TYPE-04 cycles therefore reject infinite required construction,
without mistaking empty recursive lists for eager recursion. Loading cycles and
runtime busy reentry are not edges of this graph.

The [constant graph](../../src/semantic/constants/graph.c) has one vertex per
constant declaration, not per module type. It adds each declaration's preceding
local constant and every actual constant value read. Explicit work lists follow
fresh owned-field construction and its initializers, including index-source
constants. Default/literal construction descends into declared owned fields;
cloning an already constructed constant reads that source without rerunning its
field defaults. Empty lists create no elements, while initialized elements select
their actual construction path. Repeated visits to a type have the same default
constant-read set and may be merged. An explicit mutable-reference acquisition
inside fresh shared-value construction is rejected; empty non-owning fields do
not fabricate mutable links. Native purity and resource obligations remain the
native author's contract.

Ordering repeatedly selects the least bytewise module path/declaration ordinal
whose prerequisites have all been emitted. The emitted prefix is dependency-closed
by induction. If no ready vertex exists while vertices remain, repeatedly following
an unprocessed prerequisite must revisit a vertex in the finite residual graph.
The repeated suffix is an actual directed cycle, recorded as source-located notes.
Conversely every finite acyclic residual graph has a dependency-free vertex, so
stalling is equivalent to a cycle. Selection implements CONST-02's deterministic
ready ordering independent of discovery order. Empty graphs succeed. No recursion
or heuristic depth threshold is used. With `V` vertices and `E` edges, naive exact
selection costs `O(V (V + E))` comparisons plus path-comparison bytes; graph memory
is `O(V + E)`, with at most `O(V)` additional failure notes. Failure-note allocation
can fail explicitly as out-of-memory. Every partial graph remains owned.

PROTO-07 clone inference is a separate greatest fixed point on *all* owned
interface/body scalar/list edges, including empty lists. Starting at the greatest
candidate set and monotonically removing unsupported vertices converges after at
most `V` removals. A surviving cycle is valid when it cannot reach a noncopyable
native payload; this differs from eager allocation and constant ordering.

[Relation tests](../../tests/unit/semantic/relations/main.c) cover the distinct
relations, native payload propagation, self-typed constants, owned-field constant
reads, mutable reference construction and allocation failures in both successful
ordering and cycle diagnostics. They exhaust all 512 directed graphs on three
vertices against transitive closure, checking actual diagnostic cycle edges, and
all 512 owning-list graphs with one noncopyable native seed against reachability.
The latter uses real files, the loader, ABI parser and type checker. Release/Werror
and allocation-injection tests pass; runtime busy behavior is not implemented by
this compile-time service.

## Shared storage/startup plans

`semantic_constants` consumes the successful declaration-level order once and
creates one [shared plan](../../src/semantic/constants/plan.c) per constant.
Each entry links its nominal symbol, typed initializer recipes, startup ordinal
and forward/reverse neighbors. A symbol's `shared` link makes direct and qualified
reads refer to that same storage, including typed library enumeration values.
A constants-only namespace contributes shared plans without an executable pool.
Constants are not interface/body instance fields and do not participate in copying
an instance. No native constructor runs during planning.

Induction over the already validated order proves the startup list is a bijection
with constant declarations and respects all construction dependencies. Setting
`next.previous` at insertion proves that reverse traversal is exactly reverse
startup order. Runtime initialization will track the successful prefix and clean
that prefix on failure; successful shared storage is disposed after instances,
once in reverse order. Planning does not itself initialize or destroy values.
This stage costs `O(K)` time/storage for `K` constants and never depends on C layout
sizes. Failure retains a partial owned list without setting `constants_valid`.
[Constant tests](../../tests/unit/semantic/constants/main.c) check cross-module
prerequisites, bytewise ready ordering, static reads, enum literal payloads,
constants-only namespaces, exclusion of unused files, exact reverse traversal and
every plan allocation failure. Release/Werror tests pass.

## Type diagnostics and composed frontend boundary

[Type reports](../../src/semantic/report/types.c) preserve the primary operation
span, actual/expected full module paths, and both source/destination declaration
spans with effective storage forms. Callee outputs retain their original field
identity instead of becoming anonymous values. Literal capability errors retain
the bound parameter declaration and type contract; clone capability errors also
name the native noncopyable seed reached through the owned graph. Each removed
type inherits an already removed dependency's seed/cause, so the witness follows
the monotone greatest-fixed-point elimination rather than guessing from names.
Structured reports borrow stable owned declarations/ABI texts and use fixed note
storage; reporting a type mismatch itself allocates nothing.

`semantic_analyze` composes namespace, required native, value/interface, relation
and shared-plan checks, stopping at the first failed stage. The iterative walker
carries explicit structural loop depth; `^` outside a loop is rejected even in
unreachable code, without source-offset guessing. The same frontend service can
serve the later check-only CLI and project compiler. Its success does not claim
handwritten C bodies/signatures have passed the C toolchain; those checks require
the later declaration/scaffolding/backend integration.

[Diagnostic unit tests](../../tests/unit/semantic/diagnostics/main.c) check both
nominal paths/declarations, parameter-selected literal failures and a noncopyable
native seed inside recursive composition. [Integration tests](../../tests/integration/semantic/types/main.c)
load real UTF-8 module identities and compare the complete rendered diagnostic,
including declaration excerpts and byte offsets, with a handwritten expectation.
Full `make test` with release flags, `-DNDEBUG`/`-Werror`, focused ASan/UBSan semantic,
planning and integration suites, and independent public-header checks pass.
LeakSanitizer is disabled; exhaustive allocation-injection/live-count checks
cover partial namespace, ABI, resolution, graph, constant and typed-plan owners.


T147 documents every dependency/semantic/plan/lowering file, public type and
nontrivial operation with explicit owner/borrow/failure contracts. Loading closure
uses a first-discovery requester forest; cycles reuse registered loading records.
Eager ownership is a separate finite prerequisite DAG. Shared initialization adds
actual value reads and fresh-construction reads, then stable full-key ordering.
Clone capability is a greatest fixed point over all declared owned types, including
empty-list element types. Each pass only removes capability, so at most V types
can change from true to false. The exact scans may cost O(V*(V+E)); they are not
claimed linear. Graph ordering scans ready candidates and their prerequisites;
its current repeated-scan algorithm likewise has explicit graph/key lookup costs.

A discovered COPY-06 violation had exempted noncopyable resources when receiver
and source had identical text. The semantic regression rejected that behavior,
including scalar, whole-list and indexed forms and a transitive noncopyable field.
Reference transfers still require no target clone capability. Lowering now always
retains all replacement phases and no `TRANSFER_NOOP` case exists. Runtime retains
its checked-identity ordinary no-op, which neither clones nor clears positions.
The semantic/planning regressions failed before the correction; actual generated
scalar/list/indexed self copies and invalid index conversion exercise its consumer.

This does not close T030–T040 or T041–T045. Source-order inferred fields, lexical
foreach bindings, current result trees/list services and complete control contexts
still require their own work. In particular the retained empty-loop control graph
still needs FLOW-02's one-check behavior in T044/T068; a body containing only a
declaration is syntactically nonempty under that rule. No runtime emptiness or
emission-size heuristic may choose the future repeat edge.
