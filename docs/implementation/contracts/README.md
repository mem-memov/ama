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

Entries below retain useful existing arguments and executable test locations;
they do not certify current grammar, typed list services, context or embedding
conformance. T136 removed the superseded dispatcher; T137 corrected derived
descriptions without changing the canonical requirements.
When adapting a component, update its old description rather than keeping
competing API contracts. File/interface reference pages come from Doxygen through
`make docs`; this registry records mathematical arguments and acceptance evidence.

## Before implementation

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
| Checked complete-name declarations and structural regions | [Declaration parsing](../declaration-parsing.md); shared activation/token recognition, maximal names and region closing | [Declaration/region cases](../../../tests/unit/parser/declarations/main.c), [bounded names](../../../tests/unit/lexer/tokens/main.c) |
| Draft activation and section order | [Section contract](../parser-sections.md); exact activation and increasing ranks | [Section tests](../../../tests/unit/parser/module/main.c) |
| Exact complete-name tokens and structural regions | [Lexical contract](../lexical-tokens.md); longest fixed match, compact join relation, original payloads and caller-supplied region depth | [Token boundary tests](../../../tests/unit/lexer/tokens/main.c) |
| Physical layout and literal boundaries | [Layout contract](../layout.md); prefix partition and exact payload ranges | [Layout tests](../../../tests/unit/lexer/layout/main.c) |
| Diagnostic rendering and C line mappings | [Rendering contract](../diagnostic-rendering.md); exact line coverage, byte escaping and C round trip | [Render tests](../../../tests/unit/diagnostics/render/main.c), [source composition](../../../tests/integration/source/diagnostics/main.c) |
| Borrowed structured diagnostics and source/identity adaptation | [Diagnostic contract](../diagnostics.md); location validity and explicit error partition | [Record tests](../../../tests/unit/diagnostics/record/main.c) |
| Qualified identity encoding and C names | [Identity contract](../identities.md); uniquely decodable encoding and full-key collision rejection | [Identity tests](../../../tests/unit/source/identity/main.c) |
| Exact project inputs and invalidation | [Input graph](../project-inputs.md); byte equality, counted configuration and least dependency fixed point | [Graph/capture cases](../../../tests/unit/project/inputs/main.c), [frontend composition](../../../tests/unit/compiler/check/main.c) |
| Module catalog, structural directories and companions | [Path contract](../module-paths.md); FIFO prefix induction and identity/inode injectivity | [Composed catalog tests](../../../tests/unit/source/module/main.c) |
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

Pool/execution foundations have the component evidence linked above. Their new
context, result-tree, membership/foreach and external-boundary consumers remain
open in TODO. Each future component must add its operation-level argument and
actual acceptance commands before being marked complete; neither this registry
nor a planned test-home path certifies implementation of the whole language.
