# Check-only project frontend

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

`ama --check ROOT .module` uses [CompilerFrontend](../../src/compiler/frontend.h),
the same loaded/semantic/planned project API consumed by native authoring and the
project emitter. It performs no C emission, toolchain invocation, native
execution or filesystem writes. Handwritten C bodies/signatures are checked by
later C-toolchain integration, not declared correct by check-only success.

Additional `--root DIR` values supply explicit library roots. `--exclude PATH`
applies an exact root-relative exclusion to every configured root; there are no
guessed generated-directory exclusions or configurable list role mappings.
Dependencies follow the declared source closure; repeated module identities share
one project record. Output is `checked .module` only
after all source, semantic, relation, shared-storage and control-plan stages pass.

Initialize the frontend owner, check once, inspect success or its borrowed error,
then destroy it. Owners are released in plan/semantic/project order, including
partial construction. Root/options strings need survive the operation only.
The exact command parser owns bounded arrays and borrows arguments. Invalid paths,
unknown/missing options, allocation and output errors fail explicitly. Indexing
assumes a stable filesystem under the source catalog contract.

Composition is a sequence of total stage outcomes: the first failed stage
publishes its existing diagnostic and no valid-looking result; otherwise each
stage's postcondition establishes the next one's precondition. Success therefore
means exactly that the reusable frontend accepted this project, independent of
whether a caller intends checking or later emission. Costs are the sum of the
existing source/semantic/planning costs plus linear command processing;
integer argument-array sizes are checked before allocation. No source traversal
or resource limit is guessed.

[Frontend tests](../../tests/unit/compiler/check/main.c) exercise actual component
handoffs, invalid syntax/dependencies/types/cycles and every frontend allocation
failure. [CLI tests](../../tests/integration/check/main.c) run the real executable
on native inputs containing `#error`, checking status/streams and absence of
source-root output directories. Recursive leaves retain the existing C/assert
harness. Release/Werror focused checks pass. Check-only success is deliberately
limited to this boundary; [emission](project-emission.md) separately compiles and
executes the supported subset.

## Inspecting activation without project validation

`ama --status ROOT .module` indexes the same exact roots, then parses only the
selected source and prints `draft`, `constants`, `ama` or `native`, followed by
its full logical identity. It does not resolve declarations/dependencies or
certify an executable module. In particular a draft succeeds as an inspection
but fails when selected by `--check` as the entry or a required dependency.
A valid `@C` selector can be inspected before companions exist. Syntax/UTF-8/path
errors still fail explicitly.

[ModuleInspection](../../src/compiler/inspect/module.h) owns the catalog and one
AST and destroys them in borrow-safe order. Its success partition is exactly the
parser's four activation kinds. No heuristic decides that unfinished text is
runnable. Source/catalog/parser correctness and termination compose directly;
inspection adds constant work and no output files. [Status tests](../../tests/unit/compiler/status/main.c)
cover all four kinds without dependency loading and every allocation failure;
real CLI tests distinguish inspection success from required-draft errors.

## Shared plans and native authoring

The scaffold consumes the exact `ProgramPlan` returned by strict checking, rather
than reparsing or rebuilding its own type relation. `--create` is a separate
authoring mode with a selected explicit ABI proposal; it does not weaken ordinary
checks or certify the proposed C implementation. [Scaffold acceptance](../../tests/unit/native/scaffold/declarations/main.c)
checks this identity and fault-injects the complete authoring frontend. The
project emitter consumes that same plan; real generated-project tests verify its
C compilation and native execution. Current syntax/conformance gaps remain in
the [audit](audit/README.md).
