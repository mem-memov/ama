# Component contracts and correctness arguments

This is the working procedure for ARCH-05/ARCH-06 and TEST-08 in the
[technical specification](../../../spec/README.md). It makes those
requirements usable during implementation; it does not add language semantics.
Use the [component template](component-template.md) and the completed
[driver example](driver.md).

## Current implementation boundary

T001's [audit](../audit/README.md) distinguishes passing old-subset evidence from
current conformance. T002's [component map](../components.md) assigns retained,
adapted and planned responsibilities, narrow APIs and independent/parent test
homes. The [T133 decomposition closure](../audit/decomposition-closure.md) reconciles all
maintained inputs and seven completed phase splits. The [current boundary contract](current-boundaries.md) supplies the
identity, binding, ownership, context, control and membership invariants to
preserve or implement. It also separates native publication from relocation,
and ordinary native views from compiled-component transports.

Entries below route actual component arguments and executable tests. Each
contract establishes its declared domain; complete release conformance is the
separate requirement/acceptance matrix. The
[current component map](../components.md) and T116 documentation closure
reconcile current grammar, typed lists, context and embedding consumers.
T136 removed the superseded dispatcher; canonical requirements remain unchanged.
When adapting a component, update its old description rather than keeping
competing API contracts. File/interface reference pages come from Doxygen through
`make docs`; this registry records mathematical arguments and acceptance evidence.

The [context isolation audit](../context-isolation-audit.md) records actual
context-local storage, synchronized native bookkeeping and native dependency
concurrency constraints, including overlapping constant lifecycle evidence.

The [parallel execution acceptance](../parallel-context-execution.md) checks
actual overlapping generated dispatch, context-local startup/reuse, native event
import while a peer runs, and independent continuation after peer failure.

The [executor transfer contract](../executor-transfer.md) separates core
admission, synchronized host publication and native affinity, with compiled
permanent-thread handoff and pinned-resource evidence.

The [typed message contract](../typed-messages.md) defines the explicit data-only
wire profile, allocation-free validation, independent native ownership and typed
Ama publication/failure evidence.

The [bounded channel contract](../bounded-channels.md) proves whole-message FIFO
ownership, simultaneous count/byte limits, native endpoint shutdown and independent
receiver lifetime, including real concurrent and typed Ama acceptance.

The [project configuration contract](../project-configuration.md) records the
T260 source parser, T261 path/scope foundation, T263 selector matcher,
T262 authorization and planned T232 integration obligations. Its planned
boundaries are distinct from claims of implemented compiler support.

The [prepared source protocol](../source-protocol.md) records T273’s owned
request/response boundary, exact capture equality and shell-free child admission,
with failure and real compiler handoff evidence. It supports the unfinished
[source operation integration](../source-operations.md) without claiming public
copy/delete/move support.

## Before implementation

The [declaration-expression contract](../declaration-expressions.md) records
the shared parser, source-only nominal equations and guarded selected-value
engine. It distinguishes those accepted foundations from T226's remaining
ordinary annotation, inference and runtime-admission integration.

The [T168 template audit](../audit/template-sprint.md) preserves the original
concrete-only assumptions and adaptation owners. The delivered static/ordinary
components now have [current reviewed evidence](../conformance/README.md),
[clean release results](../conformance/template-release.md) and the linked
component/consumer contracts below. Historical passing cases do not automatically
certify a future change; review its actual affected boundary and evidence.

Identify one responsibility and its smallest meaningful public boundary. Link the
applicable specification requirements and state the algorithm's complete domain.
Separate malformed supported input from caller precondition violations. State
ownership, effects, resource limits and environmental assumptions explicitly.

Choose an exact algorithm covering that domain. Derive its invariants and failure
transitions before integrating consumers. If a case contradicts the specification,
report the contradiction; do not narrow the language or modify the specification
to make the implementation easier. Ordinary toolchain assumptions are not evidence
that a source-language case is unsupported.

## During implementation

Keep a short contract beside the exposed declaration and a substantive derivation
in the component's design document. A small operation may share its meaningful
parent's contract; trivial helpers do not need artificial public interfaces or
separate proof files. Point to the actual source operations and their effects.

Use the mathematical form suited to the operation: a total function for pure
recognition, induction for a finite traversal, invariant preservation for mutable
state, or a simulation/refinement argument for lowering. Stateful/I/O operations
may be relations with several allowed outcomes, but their preconditions, effects
and status must remain explicit. Do not claim determinism for external I/O.

Derive tests from partitions, boundaries, invariants and failure transitions.
Enumerate small finite models where practical; an exhaustive claim must state its
finite domain. Test real component handoffs separately from isolated operations.
No example-specific exception, guessed interpretation, sampled acceptance or
heuristic shortcut is permitted. Tests supplement the argument, not replace it.

## Before marking a task complete

- The source boundary, documented contract and specification agree.
- Inputs, ownership, outputs and failure effects cover the declared domain.
- The derivation explains every branch/state transition and the composed handoff.
- Termination and complexity statements name their parameters and assumptions.
- Tests exercise the implemented boundary, failures and real composition; the
  document names their locations and records commands actually run.
- Remaining limits and unverified properties are explicit. A compiler/runtime
  capability is not complete merely because a mathematical sketch exists.
- Update the registry below and the affected backlog item, then commit the task.

This is a review gate, not an automatic theorem prover. The foundation task
establishes this procedure and applies it to existing components. Future components
must still supply their own arguments and executable evidence when implemented.

## Foundation registry

| Component boundary | Contract and argument | Executable evidence |
| --- | --- | --- |
| Reusable check-only frontend and CLI | [Checking contract](../checking.md); composed stage postconditions and read-only inputs | [Frontend tests](../../../tests/unit/compiler/check/main.c), [CLI tests](../../../tests/integration/check/main.c) |
| Typed lifecycle/control-flow and storage plans | [Plan contract](../typed-plans.md); separate storage domains and continuation-composition induction | [Planning tests](../../../tests/unit/planning/ir/main.c) |
| Typed entry preparation and value transfers | [Preparation](../entry-preparation.md); source leases, constructor selection and per-field commit invariants | [Calls](../../../tests/unit/runtime/calls/main.c), [independent preparation](../../../tests/unit/runtime/call-preparation/main.c), [forest copies](../../../tests/unit/runtime/transfers/main.c), [execution](../../../tests/unit/codegen/execution/main.c) |
| Stackless transfers, native views and traces | [Execution](../stackless-execution.md); explicit caller/outgoing invariant, scoped views and static source maps | [Compiled projects](../../../tests/unit/codegen/execution/main.c) |
| Constants and heap-root initialization | [Startup contract](../program-startup.md); ordered private construction, pool-free shared values and initialized-prefix cleanup | [Program tests](../../../tests/unit/runtime/program/main.c), [emitted execution](../../../tests/unit/codegen/assembly/main.c) |
| Single project C assembly | [Emission contract](../project-emission.md); shared nominal declarations, C-supplied layout and qualified SDK inclusion | [Assembly tests](../../../tests/unit/codegen/assembly/main.c), [CLI handoff](../../../tests/integration/check/main.c) |
| Ownership construction, cloning, replacement and acquisition | [Instance/ownership contract](../instance-ownership.md); prefix ledgers, injective clone maps, stable bank commit and complete subtree protection | [Lifecycle](../../../tests/unit/runtime/objects/lifecycle/main.c), [clone](../../../tests/unit/runtime/objects/clone/main.c), [replacement](../../../tests/unit/runtime/objects/replacement/main.c), [access](../../../tests/unit/runtime/objects/access/main.c) |
| Value selection and forest transfer phases | [Selection/transfer argument](../value-selection-and-list-transfer.md); explicit storage domains, exact index effects, staging ownership and whole-forest map | [Selection](../../../tests/unit/runtime/value-selection/main.c), [transfer phases](../../../tests/unit/runtime/list-transfer/main.c), [forest composition](../../../tests/unit/runtime/transfers/main.c) |
| Private clone preparation | [Clone argument](../clone-preparation.md); protected source, injective boundary map and prefix rollback | [Independent phases](../../../tests/unit/runtime/objects/clone-preparation/main.c), [clone](../../../tests/unit/runtime/objects/clone/main.c), [replacement](../../../tests/unit/runtime/objects/replacement/main.c) |
| Private construction phases | [Construction argument](../object-construction.md); registered reserved identities and exact initialized-prefix unwind | [Independent phases](../../../tests/unit/runtime/objects/construction/main.c), [lifecycle composition](../../../tests/unit/runtime/objects/lifecycle/main.c) |
| Stable instance pools | [Instance contract](../instance-ownership.md); stable blocks, private reservation, monotone generations and retirement | [Pool tests](../../../tests/unit/runtime/pool/main.c) |
| Native scaffold model, declarations and explicit publication | [Scaffold contract](../native-scaffolding.md); qualified declaration algebra, view acyclicity and staged publication | [Composed scaffold tests](../../../tests/unit/native/scaffold/declarations/main.c) |
| Semantic namespaces, values, relations and shared plans | [Semantic contract](../semantic-analysis.md); namespace invariants, nominal storage matrix, fixed points and declaration-level ordering | [Semantic suites](../../../tests/unit/semantic/Makefile), [Unicode type diagnostics](../../../tests/integration/semantic/types/main.c) |
| Required project dependency closure | [Closure contract](../project-dependencies.md); least fixed point, injective registry and finite requester traces | [Closure/model tests](../../../tests/unit/project/closure/main.c), [Unicode diagnostics](../../../tests/integration/project/closure/main.c) |
| Module-qualified native C naming | [Namespace contract](../native-namespaces.md); collision-checked module prefix and unambiguous local-token qualification | [Namespace tests](../../../tests/unit/native/namespace/main.c), [typed two-module composition](../../../tests/integration/parser/grammar/main.c) |
| Grammar expectations and frontend composition | [Error/composition contract](../parser-errors.md); deterministic grammar keys and real source/AST/native handoffs | [Error tests](../../../tests/unit/parser/errors/main.c), [access model](../../../tests/unit/parser/access/main.c), [frontend integration](../../../tests/integration/parser/grammar/main.c) |
| Fixed native ABI record parsing | [ABI parser contract](../abi-parser.md); six-slot uniqueness and explicit lexical recognition | [ABI parser tests](../../../tests/unit/parser/abi/main.c) |
| Call bindings, literal inputs and positional holes | [Call contract](../call-bindings.md); finite-state list invariants, preserved payloads and independent counters | [Call tests](../../../tests/unit/parser/calls/main.c) |
| Source AST and structural statement parsing | [AST contract](../syntax-tree.md); grammar simulation with explicit frames/access chains | [Tree tests](../../../tests/unit/parser/tree/main.c) |
| Direct result operation trees and refusal ownership | [Result parsing](../result-parsing.md); ordered action paths, explicit parser frames and caller-context handlers | [Tree/depth/failure cases](../../../tests/unit/parser/results/main.c), [safe frontend staging](../../../tests/unit/compiler/check/main.c) |
| [Control parsing](../control-parsing.md) | Branches, lexical loop roots, traversal binders and empty-body warnings | `tests/unit/parser/control`, `tests/unit/codegen/execution`, `tests/unit/driver/run` |
| Source modes and uniform assignments | [Source parsing](../source-parsing.md); disjoint source prefixes, exact indices and receiver child-region containment | [Source AST cases](../../../tests/unit/parser/sources/main.c), [safe frontend staging](../../../tests/unit/compiler/check/main.c) |
| Checked complete-name declarations and structural regions | [Declaration parsing](../declaration-parsing.md); shared activation/token recognition, maximal names and region closing | [Declaration/region cases](../../../tests/unit/parser/declarations/main.c), [bounded names](../../../tests/unit/lexer/tokens/main.c) |
| Draft activation and section order | [Section contract](../parser-sections.md); exact activation and increasing ranks | [Section tests](../../../tests/unit/parser/module/main.c) |
| Exact complete-name tokens and structural regions | [Lexical contract](../lexical-tokens.md); longest fixed match, compact join relation, original payloads and caller-supplied region depth | [Token boundary tests](../../../tests/unit/lexer/tokens/main.c) |
| Physical layout and literal boundaries | [Layout contract](../layout.md); prefix partition and exact payload ranges | [Layout tests](../../../tests/unit/lexer/layout/main.c) |
| Diagnostic rendering and C line mappings | [Rendering contract](../diagnostic-rendering.md); exact line coverage, byte escaping and C round trip | [Render tests](../../../tests/unit/diagnostics/render/main.c), [source composition](../../../tests/integration/source/diagnostics/main.c) |
| Borrowed structured diagnostics and source/identity adaptation | [Diagnostic contract](../diagnostics.md); location validity and explicit error partition | [Record tests](../../../tests/unit/diagnostics/record/main.c) |
| Qualified identity encoding and C names | [Identity contract](../identities.md); uniquely decodable encoding and full-key collision rejection | [Identity tests](../../../tests/unit/source/identity/main.c) |
| Exact project inputs and invalidation | [Input graph](../project-inputs.md); byte equality, counted configuration and least dependency fixed point | [Graph/capture cases](../../../tests/unit/project/inputs/main.c), [frontend composition](../../../tests/unit/compiler/check/main.c) |
| Module catalog, structural directories and companions | [Path contract](../module-paths.md); FIFO prefix induction and identity/inode injectivity | [Composed catalog tests](../../../tests/unit/source/module/main.c) |
| Version identity and pre-index package selection | [Versioned source foundation](../versioned-source.md); lexical fields, bounded endpoint states and local-first whole-version selection; T231 owns pending compiler integration | [Path cases](../../../tests/unit/source/path/main.c), [Package-selection and failure cases](../../../tests/unit/source/module/packages/main.c) |
| UTF-8, pinned XID and exact directory spelling | [Unicode contract](../unicode.md); scalar partition and interval membership | [Unicode tests](../../../tests/unit/source/unicode/main.c) |
| Immutable sources and byte spans | [Source storage](../source-storage.md); atomic publication and line-table predecessor invariant | [Source tests](../../../tests/unit/source/text/main.c) |
| Typed control-site lookup and C operation emission | [Control emission](../control-emission.md); exact unique IR identity, explicit inactive metadata, source provenance and terminal/successor refinement | [Standalone queries/writers](../../../tests/unit/codegen/control/main.c), [real assembly/invalid IR](../../../tests/unit/codegen/assembly/main.c), actual execution/continuation parent compositions |
| Native filesystem publication and prefix recovery | [Publication phases](../native-publication.md); narrow IO context, explicit ownership policy, descriptor/path lifetimes and changed-prefix invariant | [Filesystem children](../../../tests/unit/native/scaffold/filesystem/main.c), [real transactions](../../../tests/unit/native/scaffold/publication/main.c), actual scaffold parent and native/project CLI |
| Request recognition, response writing and driver composition | [Driver](driver.md); exact predicate, I/O effects and success law | [Request tests](../../../tests/unit/driver/request/main.c), [composite tests](../../../tests/unit/driver/run/main.c), [launcher tests](../../../tests/integration/launcher/main.c) |
| Exact CLI option recognition and owned request assembly | [Option contract](../driver-options.md); complete spelling/request relation, bounded arrays, borrowed values and publication invariant | [Independent options](../../../tests/unit/driver/options/main.c), real driver parent and check/native/emit CLI compositions |
| Source discovery, command manifests and incremental dependencies | [Incremental build](../incremental-build.md); finite traversal, private live-process candidates and exact command equality under stated Make/toolchain assumptions | [Rebuild tests](../../../tests/integration/rebuild/tracking/main.c), [controlled overlap](../../../tests/integration/rebuild/concurrent/main.c); the latter executes its pipe-controller fixture and shell helper |
| Recursive parent/leaf scheduling and assertion configuration | [Test harness](../test-harness.md); dependency ordering and induction over the test tree | [Lifecycle tests](../../../tests/integration/harness/lifecycle/main.c) |
| Suspension, ownership-wide position copying and explicit host boundaries | [Continuation contract](../continuations.md); finite saved-position domain, service projection and return-link release | [Generated continuations](../../../tests/unit/codegen/continuations/main.c), [clone](../../../tests/unit/runtime/objects/clone/main.c), [list forests](../../../tests/unit/runtime/transfers/main.c), [return transitions](../../../tests/unit/runtime/calls/main.c) |
| Published scalar library contracts and interface ordering | [Library modules](../../../stdlib/README.md); checked decimal construction, finite nominal enums, result-first scalar operations | [Compiled library compositions](../../../tests/unit/codegen/library/main.c) |
| Ordered handle lists and failure-safe mutations | [List contract](../lists.md); geometric capacity bound, ownership preflight and suffix-order preservation | [List runtime](../../../tests/unit/runtime/lists/main.c) |
| Runtime identities, service contexts and host metadata | [Metadata contract](../runtime-metadata.md); generation monotonicity, activity partitions and explicit copy exclusions | [Metadata tests](../../../tests/unit/native/metadata/control/main.c) |
| Native lifecycle ledger and failed construction/cleanup | [Lifecycle contract](../native-lifecycle.md); initialized-prefix and decreasing cleanup rank | [Resource model](../../../tests/unit/native/lifecycle/model/main.c) |
| Entry disappearance and Ama-shaped label transfers | [Toolchain contract](../native-toolchain.md); exact symbol-family validation plus required compiler enforcement | [Symbol tests](../../../tests/unit/native/toolchain/symbols/main.c), [GCC probes](../../../tests/integration/native/toolchain/main.c) |
| Native representations, status and typed hook composition | [ABI](../native-abi.md), [status](../native-status.md), [hook harness](../native-hooks.md) | [ABI suites](../../../tests/unit/native/abi/Makefile) |

Pool/execution foundations and their current context, result-tree,
membership/foreach and external consumers have separate contracts and actual
test homes in the [component map](../components.md). Operation-level arguments,
measured domains and executable evidence remain required for each new boundary;
a registry entry alone does not certify whole-language conformance.

[Parser composition](../parser-composition.md) records the T028 Ama/ABI source boundary, canonical examples and exhaustive failure checks.

[Explicit declaration inventories](../declaration-inventories.md) records complete source-ordered names, nominal linking, shared interface metadata and T030 evidence.

[Source-order inference](../source-order-inference.md) records T031 receiver classification, exact typed interfaces, foreach cells, normal field construction and compiled execution evidence.

[Live traversal execution and copying](../list-traversal-execution.md) records T084
control-label integration, exact membership translation, ordinary-copy exclusions,
candidate-potential argument and runtime/generated-project failure evidence.

[Canonical scalar library](../scalar-library.md) records T085 arbitrary-precision
construction/conversion invariants, explicit CLI root authority, full host-size
bridge, real native compilation and constructor-failure evidence.

[Failure values and text resources](../failure-values.md) records T086 coherent
condition/description publication, native share-credit algebra, UTF-8 validation,
rollback through real owned-tree cloning and separate last-refusal snapshots.

[Arithmetic and list compositions](../library-compositions.md) records T087
signed carry/borrow invariants, complete failure outcomes and real context-local
constant-list initialization, cloning and partial-cleanup evidence.

[Executable examples](../examples.md) records T088 reproducible synchronous and
continuation workflows, their lifetime arguments and exact integration evidence.

[Example usability](../example-usability.md) records T089 allocation-free copy
capability witnesses, exact source navigation and current implementation guidance.

[Explicit C-library exports](../c-library-exports.md) records T123 configuration,
checked transport profiles, complete ABI keys and exact input invalidation.

[Public C header](../c-library-header.md) records T124 typed consumer declarations,
fixed calling-convention validation and exact cross-translation-unit ABI guards.

[Task detachment](../task-detachment.md) records exact native cell retirement,
checked reference visitors and owned registration cleanup without continuation execution.

[Cooperative cancellation](../task-cancellation.md) records the terminal transition
algebra, explicit queue wakeup and late-result cleanup at documented task boundaries.

[Task copy and failure boundaries](../task-copy-and-failure.md) records explicit
snapshot parameter propagation, noncopyable enrollment ownership, external
consumer rights and native acknowledgment after terminal context cleanup.

[Supported asynchronous host](../asynchronous-host.md) records native-only creator-code
leases, real worker completion and actual unload/reload through the SDK-free C API.

[Channel readiness](../channel-waits.md) records exact registration, cancellation,
shutdown wakeups and native lifetime composed with isolated Ama tasks.

[Parallel supervision](../parallel-supervision.md) records real overlapping actor
execution, explicit failure policies and the component/native dependency matrix.

[Editor analysis](../editor-analysis.md) records immutable original-source snapshots,
exact declaration identities, lexical availability and checked interface metadata.

[Identity-aware rename](../editor-rename.md) records complete workspace discovery,
exact source edits, capture rejection and versioned client document transactions.

[Relocation bindings](../relocation-references.md) records augmented original
registries, resolved edge mapping and exact path-span edits without mutation.

[Relocation staging](../relocation-staging.md) records exact prepared bytes,
independent final graph validation, documented settings and explicit invalidations.

[Relocation application](../relocation-transaction.md) records exclusive complete-scope
revalidation, filesystem-local originals, inverse publication and retained commit receipts.


[Relocation recovery](../relocation-recovery.md) records complete persisted plan
validation, preparation intent, durable decision/cleanup and repeatable receipts.

[Source commands](../source-tool-commands.md), [relocation commands](../relocation-commands.md)
and the [debugger client](../debugger-client.md) expose those checked components
without a second parser, implicit execution or guessed outcome.

[Combined tool workflows](../tool-workflows.md) accept actual document transactions,
logical/physical relocations, historical receipt replay, native execution and fresh
verified rebuilds over one complete owned workspace.

[Specialization key encoding and registry](../static-registry.md) separates
T216's accepted normalized-key/collision-checked naming service from T183's
canonical candidate/use states, one-time local-analysis claims and exact finite
dependency closure. Actual guarded ancestry survives cache reuse, including
pending keys and every outgoing branch. Neither registration nor
a key name is executable admission.

[Static creation and definition guards](../static-guards.md) retain exact
canonical-key reachability, pending repeated scopes and separate static-value
cycle checking. T201 composes these guards with
[static result equations](../static-projections.md) and
[complete immutable publication](../static-publication.md); their component
records distinguish metadata checking from later whole-program admission.

T181's [dependent constant metadata contract](../static-constants.md) owns shared
declaration Type obligations, exact qualified-source provenance, explicit/fixed
namespace selection and typed initializer equality. Constant values, construction
capabilities and executable admission remain separate later consumers.

The [source-bound runtime field and connection contract](../static-connections.md)
owns original lexical identities, source/default/input/result equations and exact
symbolic owner congruence. Canonical intrinsic list ports and independently checked
constant namespaces supply Type facts without inventing runtime values or callee
roots. [Component suites](../../../tests/unit/semantic/static/connections/Makefile)
and finite nominal-shape/congruence queries separate these facts from T184's
actual guarded closure and T185's body/capability admission.

The [actual static metadata contract](../static-metadata.md) owns T208's
immutable actual descriptions and minimum requirements derived from complete
source use sets. It separates these requirements from complete signature
equality and from later producer/equation admission.

The [named application/equation contract](../static-equations.md) owns T209's
named map normalization and nonexpanding B-minus-P recovery. Source-only static
producers and export selection remain separate consumers, so their noninjective
outputs cannot manufacture a nominal constructor origin through this interface.

The [guarded closure contract](../static-closure.md) records [T184](../../../DONE.md#t184)'s accepted
composition of ordinary evidence and actual incoming specialization requirements.
It separates accepted Type closure from T185 ordinary/executable admission.

The [specialized ordinary semantics contract](../specialized-semantics.md) records
T185's checked projection, ordinary operations, exact frontend root selection
and finite candidate-group admission. T186–T188 own subsequent specialized
constant lifecycle, resolved planning and generated execution acceptance.

The [specialization constant contract](../specialized-constants.md) records T186's
accepted full-owner/member naming and startup/lifecycle composition, with T188's
remaining ordinary layout emission boundary stated explicitly.

The [concrete planning contract](../specialized-planning.md) records T187's
accepted semantic-to-resolved-storage/control composition, concrete lifecycle
policies and exact failure checks.

The [specialized execution contract](../specialized-execution.md) records T188's
accepted complete-owner C identities, partial/nested static composition, actual
stackless execution and complete managed-allocation refusal boundaries.

The [specialization diagnostic contract](../specialization-diagnostics.md) records
T189's accepted original-source frontend, generated-C and runtime trace compositions.

The [native template composition contract](../native-template-composition.md)
records T190's accepted concrete capability, typed-interface and lifecycle checks.

The [template source delivery contract](../template-source-delivery.md) records
T191's accepted separate-root execution and exact invalidation checks.

The [static editor binding contract](../static-editor-bindings.md) records T192's
active original-source navigation and checked-rename integration.

The [static relocation contract](../static-relocation.md) records T193
source-path classification, complete specialization mapping and actual relocation
transaction/recovery acceptance.

T194: [concrete template facades at the C boundary](../static-library-facades.md)
composes named static exports with actual package and isolated-child consumers.

T195: [generic list algorithm examples](../template-list-algorithms.md)
verify actual maintained Ama sources over the unchanged intrinsic list methods.

The [independent project-tool contract](../project-tool.md) owns T233 command,
scaffold publication and isolated experiment orchestration.

The [scoped acquisition planning contract](../acquisition-plan.md) owns T234
repository routes and operation-local coherent source records.

The [build/launcher handoff contract](../build-launcher-contracts.md) owns T244
request/closure/construction/schema/profile/artifact/stage boundaries and the
current-consumer migration ledger for T245–T257.
