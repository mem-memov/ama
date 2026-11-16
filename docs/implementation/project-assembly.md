# Single project C assembly

T056 connects the checked frontend and storage foundation to one complete C
translation unit under [STACK-01](../../spec/implementation/compiler.md#stack-01),
[STACK-04](../../spec/implementation/compiler.md#stack-04) and
[CABI-06](../../spec/implementation/native-interface.md#cabi-06).

## Assembly boundary and order

[Project generation](../../src/codegen/project/generate.c) stages declaration
artifacts before the header, execution body, launcher and compilation database.
It publishes readiness only after every artifact has complete bytes. Publication
and toolchain verification retain their separate ownership boundaries.

The [header](../../src/codegen/project/header.c) includes complete qualified
native representations and the runtime implementation exactly once through
include guards. It then emits pool storage structures, shared-value storage,
native bodies and typed lifecycle adapters, descriptors, initialization,
program metadata, borrowed views, operation tables and source locations. All
required C declarations therefore precede their use. Constants are constructed
at runtime before the explicit root; emission does not execute native code.

The [execution body](../../src/codegen/project/body.c) includes its own header.
It compiles directly as `project.c` without an implicit forced include. The
launcher includes that body to build the same single translation unit. These
are alternative ways to compile the assembly, not two objects to link together.
The runtime implementation in the retained header is likewise not a separate
public multi-object library contract.

All ordinary Ama module controls become labels inside
`ama_project_execute(AmaProjectHost *, AmaAccessMode)`. Entry admission,
startup, label dispatch, saved-position dispatch, explicit return dispatch and
host completion form one function. Native protocol adapters, runtime helpers
and disposal may remain C functions; they do not become an Ama call dispatcher.
The existing production toolchain checks native-entry boundaries and rejects
forbidden ordinary-module functions before accepting an executable.

## Why the composition is exact

Each typed plan contributes its finite control inventory once. Every emitted
control label uses its globally allocated selector, and every field/layout use
comes from the same nominal scaffold model used for isolated native interfaces.
There is no second interpretation of a native State layout or an independently
numbered copy of a source module. C checks declaration compatibility after all
nominal names and forward declarations exist. Guarded includes make either
interface/assembly include order converge on the same declarations.

The standalone body previously depended on the launcher including `project.h`
first. The explicit body include removes that implicit precondition while
preserving the launcher, generated symbols and execution behavior.

## Evidence and boundaries

`make -C tests/unit/codegen/assembly` runs
`it_compiles_the_self_contained_assembly_and_isolated_interfaces_in_either_include_order`.
Actual GCC C11 checks at O0 and O2 compile the body directly, each isolated native
interface first, and interfaces both before and after the complete assembly.
The probes check the context-parameterized function type and absence of leaked
module naming/member/error-provider macros. The backend-wide
`AMA_NATIVE_ENTRY` declaration macro intentionally remains available.

The same leaf's existing cases compile the real runtime/native assembly, execute
constant/root lifecycle ordering, exhaust emission allocation failures, and
reject corrupt required IR before publication. Its fixture includes two native
modules with equal C payload layouts, an ordinary root, a recursive reference
type and a shared native constant. Actual over-aligned layout/borrow composition
is covered by `make -C tests/unit/codegen/layouts`; enforced entry inlining and
forbidden Ama-function rejection by `make -C tests/unit/codegen/execution`.

Per-file coverage for the body emitter, assembly harness and included translation
probe is registered in the documentation coverage inventory.
[Dispatch](stackless-execution.md), [bindings](compiled-bindings.md) and
[structured execution](control-contexts.md) have their own compiled acceptance.
[Generated contexts](generated-contexts.md) connect independent storage through
the runtime context factory.
