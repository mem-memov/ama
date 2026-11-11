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
| Request recognition, response writing and driver composition | [Driver](driver.md); exact predicate, I/O effects and success law | [Request tests](../../../tests/unit/driver/request/main.c), [composite tests](../../../tests/unit/driver/run/main.c), [launcher tests](../../../tests/integration/launcher/main.c) |
| Source discovery, command manifests and incremental dependencies | [Incremental build](../incremental-build.md); finite traversal and exact command equality under stated Make/toolchain assumptions | [Rebuild tests](../../../tests/integration/rebuild/tracking/main.c) |
| Recursive parent/leaf scheduling and assertion configuration | [Test harness](../test-harness.md); dependency ordering and induction over the test tree | [Lifecycle tests](../../../tests/integration/harness/lifecycle/main.c) |
| Runtime identities, service contexts and host metadata | [Metadata contract](../runtime-metadata.md); generation monotonicity, activity partitions and explicit copy exclusions | [Metadata tests](../../../tests/unit/native/metadata/control/main.c) |
| Native lifecycle ledger and failed construction/cleanup | [Lifecycle contract](../native-lifecycle.md); initialized-prefix and decreasing cleanup rank | [Resource model](../../../tests/unit/native/lifecycle/model/main.c) |
| Entry disappearance and Ama-shaped label transfers | [Toolchain contract](../native-toolchain.md); exact symbol-family validation plus required compiler enforcement | [Symbol tests](../../../tests/unit/native/toolchain/symbols/main.c), [GCC probes](../../../tests/integration/native/toolchain/main.c) |
| Native representations, status and typed hook composition | [ABI](../native-abi.md), [status](../native-status.md), [hook harness](../native-hooks.md) | [ABI suites](../../../tests/unit/native/abi/Makefile) |

Source-manager, language parsing, typed IR and full pool/execution contracts will
be added only as those components are implemented. The registry does not certify
those future components or the language as a whole.
