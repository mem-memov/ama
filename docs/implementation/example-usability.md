# Usability review from executable examples

T089 uses actual failures encountered while publishing
[the executable examples](examples.md). It changes diagnostic metadata and
navigation, not language semantics. No language change is proposed by this review.

## Why a whole simulation cannot be copied

The first simulation contained a native output observer without a declared `=`
binding. `% simulation` failed correctly, but the operation and native ABI note
left the intermediate owned field implicit. A reader had to reconstruct that
ownership dependency by hand. The observer is resource-free, so the complete
example now declares and implements its legitimate clone explicitly.

The maintained negative example
[`copy failure`](<../../tests/integration/examples/project/copy failure>) instead
owns an [`uncopyable simulation`](<../../tests/integration/examples/project/uncopyable simulation>)
whose `probe` field has no copy binding. Checking this entry now reports:

1. The `% source` operation and its destination declaration.
2. The exact owned `probe` declaration in the wrapper.
3. The native `open.abi` contract lacking `=`.

The cause explains the missing declared capability. The help explains that all
owned native descendants need `=`, while copying a reference does not copy its
target. It does not invent a clone implementation, silently replace ownership
with references, or suggest that a resource is independently cloneable.

## Metadata and proof

[PROTO-07](../../spec/implementation/native-interface.md#proto-07) applies copy
capability transitively through owned fields, including empty lists. The existing
greatest-fixed-point calculation records `clone_cause` and `clone_seed`. Each
removal points to a previously excluded type, so the finite witness terminates
at the exact noncopyable native seed. Reporting borrows its first ownership edge
and final ABI source; it performs no second search, source-name inference or
allocation. Reference edges remain outside this relation.

The bounded related-location array holds at most three entries: optional
destination, optional ownership witness, and native contract. A direct missing
capability needs only the destination and native contract when a destination is present.
Literal/condition/index diagnostics keep their original behavior. All spans borrow
loaded sources through the existing semantic owner's lifetime, so CLI rendering
and later editor consumers use the same UTF-8 byte positions and physical paths.
[COMP-04](../../spec/implementation/compiler.md#comp-04) governs this source context.

## Repeated workflows and navigation

The root README now points to current examples and task acceptance evidence.
The original audit remains historical evidence rather than the advertised limit
of today's frontend. Example commands name their standard root, output directory
and SDK explicitly. Logical names containing spaces are quoted. The file example
states its working-directory requirement, preventing an unrelated missing-path
error during manual reproduction.

Compiler/native failure, refused operation, ordinary application failure, ordinary
copy and continuation copy have separate executable examples. An empty traversal
has a complete no-output case. The guide connects each effect with a canonical
rule, while Doxygen verifies rendered rule destinations and purpose comments.

## Per-file verification

- `src/semantic/report/types.c`, `src/semantic/semantic.h` and
  `src/semantic/internal.h`: the named
  `it_retains_the_owned_copy_witness_and_native_seed` case in
  `tests/unit/semantic/types/support/capabilities.c`, registered by its leaf main.
  It covers owned scalar, empty owned list, multi-edge ownership, direct native
  failure and reference exclusion. Run `make -C tests/unit/semantic/types`.
  The semantic subtree checks existing error partitions and partial-allocation
  cleanup; standalone header compilation is part of `make docs-complete`.
- The new test support file and updated main are compiled and executed by that
  leaf, with assertions enabled; ASan/UBSan exercises the same component boundary.
- `tests/integration/examples/project/copy failure`,
  `tests/integration/examples/project/uncopyable simulation` and updated main:
  `it_explains_the_transitive_copy_failure_with_source_navigation` checks the
  real CLI's operation, field and ABI line/column locations, cause and help.
  `make -C tests/integration/examples` also reruns the successful examples and
  target-sanitized native/runtime execution.
- `tests/integration/Makefile`: root integration dispatch includes this actual
  leaf; its existing build-harness coverage remains applicable.

The all-project strict suite passed immediately before this diagnostic-only task
in T088. T089 reruns the affected semantic subtree, real CLI examples, standalone
headers, per-file registry and reproducible Doxygen. Runtime execution semantics
and public language requirements are unchanged.
