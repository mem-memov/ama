# Component contracts and correctness arguments

This is the working procedure for ARCH-05/ARCH-06 and TEST-08 in the
[technical specification](../../../TECHNICAL_SPECIFICATION.md). It makes those
requirements usable during implementation; it does not add language semantics.
Use the [component template](component-template.md) and the completed
[driver example](driver.md).

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
| Ownership construction, cloning, replacement and acquisition | [Instance/ownership contract](../instance-ownership.md); prefix ledgers, injective clone maps, stable bank commit and complete subtree protection | [Lifecycle](../../../tests/unit/runtime/objects/lifecycle/main.c), [clone](../../../tests/unit/runtime/objects/clone/main.c), [replacement](../../../tests/unit/runtime/objects/replacement/main.c), [access](../../../tests/unit/runtime/objects/access/main.c) |
| Stable instance pools | [Instance contract](../instance-ownership.md); stable blocks, private reservation, monotone generations and retirement | [Pool tests](../../../tests/unit/runtime/pool/main.c) |
| Native scaffold model, declarations and explicit publication | [Scaffold contract](../native-scaffolding.md); qualified declaration algebra, view acyclicity and staged publication | [Composed scaffold tests](../../../tests/unit/native/scaffold/declarations/main.c) |
| Semantic namespaces, values, relations and shared plans | [Semantic contract](../semantic-analysis.md); namespace invariants, nominal storage matrix, fixed points and declaration-level ordering | [Semantic suites](../../../tests/unit/semantic/Makefile), [Unicode type diagnostics](../../../tests/integration/semantic/types/main.c) |
| Required project dependency closure | [Closure contract](../project-dependencies.md); least fixed point, injective registry and finite requester traces | [Closure/model tests](../../../tests/unit/project/closure/main.c), [Unicode diagnostics](../../../tests/integration/project/closure/main.c) |
| Module-qualified native C naming | [Namespace contract](../native-namespaces.md); collision-checked module prefix and unambiguous local-token qualification | [Namespace tests](../../../tests/unit/native/namespace/main.c), [typed two-module composition](../../../tests/integration/parser/grammar/main.c) |
| Grammar expectations and frontend composition | [Error/composition contract](../parser-errors.md); deterministic grammar keys and real source/AST/native handoffs | [Error tests](../../../tests/unit/parser/errors/main.c), [access model](../../../tests/unit/parser/access/main.c), [frontend integration](../../../tests/integration/parser/grammar/main.c) |
| Fixed native ABI record parsing | [ABI parser contract](../abi-parser.md); six-slot uniqueness and explicit lexical recognition | [ABI parser tests](../../../tests/unit/parser/abi/main.c) |
| Call bindings, literal inputs and positional holes | [Call contract](../call-bindings.md); finite-state list invariants, preserved payloads and independent counters | [Call tests](../../../tests/unit/parser/calls/main.c) |
| Source AST and structural statement parsing | [AST contract](../syntax-tree.md); grammar simulation with explicit frames/access chains | [Tree tests](../../../tests/unit/parser/tree/main.c) |
| Draft activation and section order | [Section contract](../parser-sections.md); exact activation and increasing ranks | [Section tests](../../../tests/unit/parser/module/main.c) |
| Physical layout and literal boundaries | [Layout contract](../layout.md); prefix partition and exact payload ranges | [Layout tests](../../../tests/unit/lexer/layout/main.c) |
| Diagnostic rendering and C line mappings | [Rendering contract](../diagnostic-rendering.md); exact line coverage, byte escaping and C round trip | [Render tests](../../../tests/unit/diagnostics/render/main.c), [source composition](../../../tests/integration/source/diagnostics/main.c) |
| Borrowed structured diagnostics and source/identity adaptation | [Diagnostic contract](../diagnostics.md); location validity and explicit error partition | [Record tests](../../../tests/unit/diagnostics/record/main.c) |
| Qualified identity encoding and C names | [Identity contract](../identities.md); uniquely decodable encoding and full-key collision rejection | [Identity tests](../../../tests/unit/source/identity/main.c) |
| Module catalog, structural directories and companions | [Path contract](../module-paths.md); FIFO prefix induction and identity/inode injectivity | [Composed catalog tests](../../../tests/unit/source/module/main.c) |
| UTF-8, pinned XID and exact directory spelling | [Unicode contract](../unicode.md); scalar partition and interval membership | [Unicode tests](../../../tests/unit/source/unicode/main.c) |
| Immutable sources and byte spans | [Source storage](../source-storage.md); atomic publication and line-table predecessor invariant | [Source tests](../../../tests/unit/source/text/main.c) |
| Request recognition, response writing and driver composition | [Driver](driver.md); exact predicate, I/O effects and success law | [Request tests](../../../tests/unit/driver/request/main.c), [composite tests](../../../tests/unit/driver/run/main.c), [launcher tests](../../../tests/integration/launcher/main.c) |
| Source discovery, command manifests and incremental dependencies | [Incremental build](../incremental-build.md); finite traversal and exact command equality under stated Make/toolchain assumptions | [Rebuild tests](../../../tests/integration/rebuild/tracking/main.c) |
| Recursive parent/leaf scheduling and assertion configuration | [Test harness](../test-harness.md); dependency ordering and induction over the test tree | [Lifecycle tests](../../../tests/integration/harness/lifecycle/main.c) |
| Runtime identities, service contexts and host metadata | [Metadata contract](../runtime-metadata.md); generation monotonicity, activity partitions and explicit copy exclusions | [Metadata tests](../../../tests/unit/native/metadata/control/main.c) |
| Native lifecycle ledger and failed construction/cleanup | [Lifecycle contract](../native-lifecycle.md); initialized-prefix and decreasing cleanup rank | [Resource model](../../../tests/unit/native/lifecycle/model/main.c) |
| Entry disappearance and Ama-shaped label transfers | [Toolchain contract](../native-toolchain.md); exact symbol-family validation plus required compiler enforcement | [Symbol tests](../../../tests/unit/native/toolchain/symbols/main.c), [GCC probes](../../../tests/integration/native/toolchain/main.c) |
| Native representations, status and typed hook composition | [ABI](../native-abi.md), [status](../native-status.md), [hook harness](../native-hooks.md) | [ABI suites](../../../tests/unit/native/abi/Makefile) |

Full pool/execution contracts will be added only as those components are implemented. The registry does not certify
those future components or the language as a whole.
